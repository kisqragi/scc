#include "scc.h"
#include <stdio.h>

static int depth;

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

static void gen_addr(Node *node) {
    if (node->kind == ND_VAR) {
        printf("    lea -%d(%%rbp), %%rax\n", node->var->offset);
        return;
    }

    error_tok(node->tok, "not an lvalue");
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
            printf("    mov (%%rax), %%rax\n");
            return;
        case ND_ASSIGN:
            gen_addr(node->lhs);
            push();
            gen_expr(node->rhs);
            pop("%rdi");
            printf("    mov %%rax, (%%rdi)\n");
            return;
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
            printf("    jmp .L.return\n");
            return;
        case ND_EXPR_STMT:
            gen_expr(node->lhs);
            return;
        default:
            error_tok(node->tok, "invalid statement");
    }
}

static void assign_lvar_offset(Function *prog) {
    int offset = 0;
    for (Obj *var = prog->locals; var; var = var->next) {
        offset += 8;
        var->offset = offset;
    }
    prog->stack_size = align_to(offset, 16);
}

void codegen(Function *prog) {
    assign_lvar_offset(prog);

    printf(".globl main\n");
    printf("main:\n");

    // Prologue
    printf("    push %%rbp\n");
    printf("    mov %%rsp, %%rbp\n");
    printf("    sub $%d, %%rsp\n", prog->stack_size);

    gen_stmt(prog->body);
    assert(depth == 0);

    printf(".L.return:\n");
    printf("    mov %%rbp, %%rsp\n");
    printf("    pop %%rbp\n");
    printf("    ret\n");
} 
