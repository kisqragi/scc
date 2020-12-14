#include "scc.h"

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
static char *argreg8[]  = {"%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
static char *argreg16[] = {"%di", "%si", "%dx", "%cx", "%r8w", "%r9w"};
static char *argreg32[] = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};
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
int align_to(int n, int align) { return (n + align - 1) / align * align; }

// Load a value from where %rax is pointing to.
static void load(Type *ty) {
    if (ty->kind == TY_ARRAY || ty->kind == TY_STRUCT) { return; }

    switch (ty->size) {
        case 1: println("    movsbl (%%rax), %%eax"); break;
        case 2: println("    movswl (%%rax), %%eax"); break;
        case 4: println("    movsxd (%%rax), %%rax"); break;
        default: println("    mov (%%rax), %%rax"); break;
    }
}

// Store %rax to an address that the stack top is pointing to.
static void store(Type *ty) {
    pop("%rdi");

    if (ty->kind == TY_STRUCT) {
        for (int i = 0; i < ty->size; i++) {
            println("    mov %d(%%rax), %r8b", i);
            println("    mov %r8b, %d(%%rdi)", i);
        }
        return;
    }

    switch (ty->size) {
        case 1: println("    mov %%al, (%%rdi)"); break;
        case 2: println("    mov %%ax, (%%rdi)"); break;
        case 4: println("    mov %%eax, (%%rdi)"); break;
        default: println("    mov %%rax, (%%rdi)"); break;
    }
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
            if (node->ty->kind == TY_VOID)
                error_tok(node->tok, "invalid use of void expression");
            gen_expr(node->lhs);
            return;
        case ND_MEMBER:
            gen_addr(node->lhs);
            println("    add $%d, %%rax", node->member->offset);
            return;
        default: error_tok(node->tok, "not an lvalue");
    }
}

static int get_typeid(Type *ty) {
    enum { I8, I16, I32, I64 };
    switch (ty->kind) {
        case TY_CHAR: return I8;
        case TY_SHORT: return I16;
        case TY_INT: return I32;
        default: return I64;
    }
}

static char i32i8[]  = "movsbl %al, %eax";
static char i32i16[] = "movswl %ax, %eax";
static char i32i64[] = "movsxd %eax, %rax";
static char i64i32[] = "cdqe";

static char *cast_table[][4] = {
    // i8    i16     i32     i64
    {NULL, NULL, NULL, i32i64},    // i8
    {i32i8, NULL, NULL, i32i64},   // i16
    {i32i8, i32i16, NULL, i32i64}, // i32
    {i32i8, i32i16, i64i32, NULL}, // i64
};

static void cast(Type *from, Type *to) {
    if (to->kind == TY_VOID) return;

    int t1 = get_typeid(from);
    int t2 = get_typeid(to);

    if (cast_table[t1][t2]) println("    %s", cast_table[t1][t2]);
}

static void gen_expr(Node *node) {
    println("    .loc 1 %d", node->tok->line_no);

    switch (node->kind) {
        case ND_NUM: println("    mov $%ld, %%rax", node->val); return;
        case ND_NEG:
            gen_expr(node->lhs);
            println("    neg %%rax");
            return;
        case ND_VAR:
            gen_addr(node);
            load(node->ty);
            return;
        case ND_DEREF:
            if (node->ty->kind == TY_VOID)
                error_tok(node->tok, "invalid use of void expression");
            gen_expr(node->lhs);
            load(node->ty);
            return;
        case ND_ADDR: gen_addr(node->lhs); return;
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
        case ND_CAST:
            gen_expr(node->lhs);
            cast(node->lhs->ty, node->ty);
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

            for (int i = nargs - 1; i >= 0; i--)
                pop(argreg64[i]);

            println("    mov $0, %%rax");
            println("    call %s", node->funcname);
            return;
        }
        default: break;
    }

    gen_expr(node->rhs);
    push();
    gen_expr(node->lhs);
    pop("%rdi");

    char *ax, *di;

    if (node->lhs->ty->kind == TY_LONG || node->rhs->ty->kind == TY_LONG ||
        node->lhs->ty->base || node->rhs->ty->base) {
        ax = "%rax";
        di = "%rdi";
    } else {
        ax = "%eax";
        di = "%edi";
    }

    switch (node->kind) {
        case ND_ADD: println("    add %s, %s", di, ax); return;
        case ND_SUB: println("    sub %s, %s", di, ax); return;
        case ND_MUL: println("    imul %s, %s", di, ax); return;
        case ND_DIV:
            if (node->lhs->ty->size == 8 || node->rhs->ty->size == 8)
                println("    cqo");
            else
                println("    cdq");
            println("    idiv %s", di);
            return;
        case ND_EQ:
        case ND_NE:
        case ND_LT:
        case ND_LE:
            println("    cmp %s, %s", di, ax);

            if (node->kind == ND_EQ) println("    sete %%al");
            if (node->kind == ND_NE) println("    setne %%al");
            if (node->kind == ND_LT) println("    setl %%al");
            if (node->kind == ND_LE) println("    setle %%al");

            println("    movzb %%al, %%rax");
            return;
        default: error_tok(node->tok, "invalid expression");
    }
}

static void gen_stmt(Node *node) {
    println("    .loc 1 %d", node->tok->line_no);

    switch (node->kind) {
        case ND_IF: {
            int c = count();
            gen_expr(node->cond);
            println("    cmp $0, %%rax");
            println("    je .L.if.else.%d", c);
            gen_stmt(node->then);
            println("    jmp .L.if.end.%d", c);
            println(".L.if.else.%d:", c);
            if (node->els) gen_stmt(node->els);
            println(".L.if.end.%d:", c);
            return;
        }
        case ND_FOR: {
            int c = count();
            if (node->init) gen_stmt(node->init);
            println(".L.for.begin.%d:", c);
            if (node->cond) {
                gen_expr(node->cond);
                println("    cmp $0, %%rax");
                println("    je .L.for.end.%d", c);
            }
            gen_stmt(node->then);
            if (node->inc) { gen_expr(node->inc); }
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
        case ND_EXPR_STMT: gen_expr(node->lhs); return;
        default: error_tok(node->tok, "invalid statement");
    }
}

static void assign_lvar_offset(Obj *prog) {
    for (Obj *fn = prog; fn; fn = fn->next) {
        // Global variable
        if (!fn->is_function) continue;

        int offset = 0;
        for (Obj *var = fn->locals; var; var = var->next) {
            offset += var->ty->size;
            offset      = align_to(offset, var->ty->align);
            var->offset = offset;
        }
        fn->stack_size = align_to(offset, 16);
    }
}

static void emit_data(Obj *prog) {
    for (Obj *var = prog; var; var = var->next) {
        if (var->is_function) continue;

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
        if (!fn->is_function || fn->is_declaration) continue;

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
            switch (var->ty->size) {
                case 1:
                    println("    mov %s, -%d(%%rbp)", argreg8[i++],
                            var->offset);
                    break;
                case 2:
                    println("    mov %s, -%d(%%rbp)", argreg16[i++],
                            var->offset);
                    break;
                case 4:
                    println("    mov %s, -%d(%%rbp)", argreg32[i++],
                            var->offset);
                    break;
                default:
                    println("    mov %s, -%d(%%rbp)", argreg64[i++],
                            var->offset);
                    break;
            }
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
