#include "scc.h"
#include <stdio.h>

static void gen_expr(Node *node);

static int depth;
static Obj *current_fn;
static char *argreg[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

static void push(void) {
    printf("    push %%rax\n");
    depth++;
}

static void pop(char *arg) {
    printf("    pop %s\n", arg);
    depth--;
}

static int count(void) {
    static int i = 1;
    return i++;
}

// Round up `n` to the nearest multiple of `align`.
// For instance, align_to(5, 8) returns 8 and align_to(11, 8) returns 16.
static int align_to(int n, int align) {
    return (n + align - 1) / align * align;
}

// Load a value from where %rax is pointing to.
static void load(Type *ty) {
    if (ty->kind == TY_ARRAY) {
        return;
    }
    printf("    mov (%%rax), %%rax\n");
}

// Store %rax to an address that the stack top is pointing to.
static void store(void) {
    pop("%rdi");
    printf("    mov %%rax, (%%rdi)\n");
}

static void gen_addr(Node *node) {
    switch (node->kind) {
        case ND_VAR:
            printf("    lea -%d(%%rbp), %%rax\n", node->var->offset);
            return;
        case ND_DEREF:
            gen_expr(node->lhs);
            return;
        default:
            error_tok(node->tok, "not an lvalue");
    }
}

static void gen_expr(Node *node) {
    switch (node->kind) {
        case ND_NUM:
            printf("    mov $%d, %%rax\n", node->val);
            return;
        case ND_NEG:
            gen_expr(node->lhs);
            printf("    neg %%rax\n");
            return;
        case ND_VAR:
            gen_addr(node);
            load(node->ty);
            return;
        case ND_DEREF:
            gen_expr(node->lhs);
            load(node->ty);
            return;
        case ND_ADDR:
            gen_addr(node->lhs);
            return;
        case ND_ASSIGN:
            gen_addr(node->lhs);
            push();
            gen_expr(node->rhs);
            store();
            return;
        case ND_FUNCALL: {
            int nargs = 0;
            for (Node *arg = node->args; arg; arg = arg->next) {
                gen_expr(arg);
                push();
                nargs++; 
            }

            for (int i = nargs-1; i >= 0; i--)
                pop(argreg[i]);

            printf("    mov $0, %%rax\n");
            printf("    call %s\n", node->funcname);
            return;
        }
        default:
            break;
    }

    gen_expr(node->rhs);
    push();
    gen_expr(node->lhs);
    pop("%rdi");

    switch (node->kind) {
        case ND_ADD:
            printf("    add %%rdi, %%rax\n");
            return;
        case ND_SUB:
            printf("    sub %%rdi, %%rax\n");
            return;
        case ND_MUL:
            printf("    imul %%rdi, %%rax\n");
            return;
        case ND_DIV:
            printf("    cqo\n");
            printf("    idiv %%rdi\n");
            return;
        case ND_EQ:
        case ND_NE:
        case ND_LT:
        case ND_LE:
            printf("    cmp %%rdi, %%rax\n");

            if (node->kind == ND_EQ)
                printf("    sete %%al\n");
            if (node->kind == ND_NE)
                printf("    setne %%al\n");
            if (node->kind == ND_LT)
                printf("    setl %%al\n");
            if (node->kind == ND_LE)
                printf("    setle %%al\n");

            printf("    movzb %%al, %%rax\n");
            return;
        default:
            error_tok(node->tok, "invalid expression");
    }
}

static void gen_stmt(Node *node) {
    switch (node->kind) {
        case ND_IF: {
            int c = count();
            gen_expr(node->cond);
            printf("    cmp $0, %%rax\n");
            printf("    je .L.if.else.%d\n", c);
            gen_stmt(node->then);
            printf("    jmp .L.if.end.%d\n", c);
            printf(".L.if.else.%d:\n", c);
            if (node->els)
                gen_stmt(node->els);
            printf(".L.if.end.%d:\n", c);
            return;
        }
        case ND_FOR: {
            int c = count();
            if (node->init)
                gen_stmt(node->init);
            printf(".L.for.begin.%d:\n", c);
            if (node->cond) {
                gen_expr(node->cond);
                printf("    cmp $0, %%rax\n");
                printf("    je .L.for.end.%d\n", c);
            }
            gen_stmt(node->then);
            if (node->inc) {
                gen_expr(node->inc);
            }
            printf("    jmp .L.for.begin.%d\n", c);
            printf(".L.for.end.%d:\n", c);
            return;
        }
        case ND_BLOCK:
            for (Node *n = node->body; n; n = n->next)
                gen_stmt(n);
            return;
        case ND_RETURN:
            gen_expr(node->lhs);
            printf("    jmp .L.return.%s\n", current_fn->name);
            return;
        case ND_EXPR_STMT:
            gen_expr(node->lhs);
            return;
        default:
            error_tok(node->tok, "invalid statement");
    }
}

static void assign_lvar_offset(Obj *prog) {
    for (Obj *fn = prog; fn; fn = fn->next) {
        // Global variable
        if (!fn->is_function)
            continue;

        int offset = 0;
        for (Obj *var = fn->locals; var; var = var->next) {
            offset += var->ty->size;
            var->offset = offset;
        }
        fn->stack_size = align_to(offset, 16);
    }
}

void codegen(Obj *prog) {
    assign_lvar_offset(prog);

    for (Obj *fn = prog; fn; fn = fn->next) {
        // Global variable
        if (!fn->is_function)
            continue;

        printf(".globl %s\n", fn->name);
        printf(".text\n");
        printf("%s:\n", fn->name);
        current_fn = fn;

        // Prologue
        printf("    push %%rbp\n");
        printf("    mov %%rsp, %%rbp\n");
        printf("    sub $%d, %%rsp\n", fn->stack_size);

        // Save passed-by-register arguments to the stack
        int i = 0;
        for (Obj *var = fn->params; var; var = var->next)
            printf("    mov %s, -%d(%%rbp)\n", argreg[i++], var->offset);

        // Emit code
        gen_stmt(fn->body);
        assert(depth == 0);

        // Epilogue
        printf(".L.return.%s:\n", fn->name);
        printf("    mov %%rbp, %%rsp\n");
        printf("    pop %%rbp\n");
        printf("    ret\n");
    }
} 
