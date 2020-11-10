#include "scc.h"
#include <stdarg.h>
#include <stdio.h>

static void gen_expr(Node *node);
static void gen_stmt(Node *node);

static FILE *output_file;
static void println(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(output_file, fmt, ap);
    va_end(ap);
    fprintf(output_file, "\n");
}

static int depth;
static Obj *current_fn;
static char *argreg8[] = {"%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
static char *argreg64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

static void push(void) {
    println("    push %%rax");
    depth++;
}

static void pop(char *arg) {
    println("    pop %s", arg);
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

    if (ty->size == 1)
        println("    movsbq (%%rax), %%rax");
    else
        println("    mov (%%rax), %%rax");
}

// Store %rax to an address that the stack top is pointing to.
static void store(Type *ty) {
    pop("%rdi");

    if (ty->size == 1)
        println("    mov %%al, (%%rdi)");
    else
        println("    mov %%rax, (%%rdi)");
}

static void gen_addr(Node *node) {
    switch (node->kind) {
        case ND_VAR:
            if (node->var->is_local) {
                // Local Variable
                println("    lea -%d(%%rbp), %%rax", node->var->offset);
            } else {
                // Global Variable
                println("    lea %s(%%rip), %%rax", node->var->name);
            }
            return;
        case ND_DEREF:
            gen_expr(node->lhs);
            return;
        case ND_MEMBER:
            gen_addr(node->lhs);
            println("   add $%d, %%rax", node->member->offset);
            return;
        default:
            error_tok(node->tok, "not an lvalue");
    }
}

static void gen_expr(Node *node) {
    println("   .loc 1 %d", node->tok->line_no);

    switch (node->kind) {
        case ND_NUM:
            println("    mov $%d, %%rax", node->val);
            return;
        case ND_NEG:
            gen_expr(node->lhs);
            println("    neg %%rax");
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
            store(node->ty);
            return;
        case ND_COMMA:
            gen_expr(node->lhs);
            gen_expr(node->rhs);
            return;
        case ND_MEMBER:
            gen_addr(node);
            load(node->member->ty);
            return;
        case ND_STMT_EXPR:
            for (Node *n = node->body; n; n = n->next)
                gen_stmt(n);
            return;
        case ND_FUNCALL: {
            int nargs = 0;
            for (Node *arg = node->args; arg; arg = arg->next) {
                gen_expr(arg);
                push();
                nargs++; 
            }

            for (int i = nargs-1; i >= 0; i--)
                pop(argreg64[i]);

            println("    mov $0, %%rax");
            println("    call %s", node->funcname);
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
            println("    add %%rdi, %%rax");
            return;
        case ND_SUB:
            println("    sub %%rdi, %%rax");
            return;
        case ND_MUL:
            println("    imul %%rdi, %%rax");
            return;
        case ND_DIV:
            println("    cqo");
            println("    idiv %%rdi");
            return;
        case ND_EQ:
        case ND_NE:
        case ND_LT:
        case ND_LE:
            println("    cmp %%rdi, %%rax");

            if (node->kind == ND_EQ)
                println("    sete %%al");
            if (node->kind == ND_NE)
                println("    setne %%al");
            if (node->kind == ND_LT)
                println("    setl %%al");
            if (node->kind == ND_LE)
                println("    setle %%al");

            println("    movzb %%al, %%rax");
            return;
        default:
            error_tok(node->tok, "invalid expression");
    }
}

static void gen_stmt(Node *node) {
    println("   .loc 1 %d", node->tok->line_no);

    switch (node->kind) {
        case ND_IF: {
            int c = count();
            gen_expr(node->cond);
            println("    cmp $0, %%rax");
            println("    je .L.if.else.%d", c);
            gen_stmt(node->then);
            println("    jmp .L.if.end.%d", c);
            println(".L.if.else.%d:", c);
            if (node->els)
                gen_stmt(node->els);
            println(".L.if.end.%d:", c);
            return;
        }
        case ND_FOR: {
            int c = count();
            if (node->init)
                gen_stmt(node->init);
            println(".L.for.begin.%d:", c);
            if (node->cond) {
                gen_expr(node->cond);
                println("    cmp $0, %%rax");
                println("    je .L.for.end.%d", c);
            }
            gen_stmt(node->then);
            if (node->inc) {
                gen_expr(node->inc);
            }
            println("    jmp .L.for.begin.%d", c);
            println(".L.for.end.%d:", c);
            return;
        }
        case ND_BLOCK:
            for (Node *n = node->body; n; n = n->next)
                gen_stmt(n);
            return;
        case ND_RETURN:
            gen_expr(node->lhs);
            println("    jmp .L.return.%s", current_fn->name);
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

static void emit_data(Obj *prog) {
    for (Obj *var = prog; var; var = var->next) {
        if (var->is_function)
            continue;

        println("    .data");
        println("    .globl %s", var->name);
        println("%s:", var->name);

        if (var->init_data) {
            for (int i = 0; i < var->ty->size; i++)
                println("    .byte %d", var->init_data[i]);
        } else {
            println("    .zero %d", var->ty->size);
        }
    }
}

static void emit_text(Obj *prog) {
    for (Obj *fn = prog; fn; fn = fn->next) {
        // Global variable
        if (!fn->is_function)
            continue;

        println("    .globl %s", fn->name);
        println("    .text");
        println("%s:", fn->name);
        current_fn = fn;

        // Prologue
        println("    push %%rbp");
        println("    mov %%rsp, %%rbp");
        println("    sub $%d, %%rsp", fn->stack_size);

        // Save passed-by-register arguments to the stack
        int i = 0;
        for (Obj *var = fn->params; var; var = var->next) {
            if (var->ty->size == 1)
                println("    mov %s, -%d(%%rbp)", argreg8[i++], var->offset);
            else
                println("    mov %s, -%d(%%rbp)", argreg64[i++], var->offset);
        }

        // Emit code
        gen_stmt(fn->body);
        assert(depth == 0);

        // Epilogue
        println(".L.return.%s:", fn->name);
        println("    mov %%rbp, %%rsp");
        println("    pop %%rbp");
        println("    ret");
    }
}

void codegen(Obj *prog, FILE *out) {
    output_file = out;
    assign_lvar_offset(prog);
    emit_data(prog);
    emit_text(prog);
} 
