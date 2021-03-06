#include "scc.h"

typedef struct VarScope VarScope;
struct VarScope {
    VarScope *next;
    char *name;
    Obj *var;
    Type *type_def;
};

typedef struct {
    bool is_typedef;
} VarAttr;

typedef struct TagScope TagScope;
struct TagScope {
    TagScope *next;
    char *name;
    Type *ty;
};

typedef struct Scope Scope;
struct Scope {
    Scope *next;
    VarScope *vsc;
    TagScope *tsc;
};

static Scope *scope;

static void enter_scope() {
    Scope *sc = calloc(1, sizeof(Scope));
    sc->next  = scope;
    scope     = sc;
}

static void leave_scope() { scope = scope->next; }

static VarScope *push_scope(char *name) {
    VarScope *vsc = calloc(1, sizeof(VarScope));
    vsc->name     = name;
    vsc->next     = scope->vsc;
    scope->vsc    = vsc;
    return vsc;
}

static void push_tag_scope(char *name, Type *ty) {
    TagScope *tsc = calloc(1, sizeof(TagScope));
    tsc->name     = name;
    tsc->ty       = ty;
    tsc->next     = scope->tsc;
    scope->tsc    = tsc;
}

// All local variable instances created during parsing are accumulated to this
// list.
static Obj *locals;
static Obj *globals;

static Obj *current_fn;

static Type *declspec(Token **rest, Token *tok, VarAttr *attr);
static Type *declarator(Token **rest, Token *tok, Type *ty);
static Node *declaration(Token **rest, Token *tok, Type *basety);
static Node *compound_stmt(Token **rest, Token *tok);
static Node *stmt(Token **rest, Token *tok);
static Node *expr_stmt(Token **rest, Token *tok);
static Node *expr(Token **rest, Token *tok);
static Node *assign(Token **rest, Token *tok);
static Node *equality(Token **rest, Token *tok);
static Node *relational(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *cast(Token **rest, Token *tok);
static Node *unary(Token **rest, Token *tok);
static Node *postfix(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);

static char *get_ident(Token *tok);
static bool is_typename(Token *tok);
static Token *parse_typedef(Token *tok, Type *basety);

// find a local variable by name.
static VarScope *find_var(Token *tok) {
    for (Scope *sc = scope; sc; sc = sc->next) {
        for (VarScope *vsc = sc->vsc; vsc; vsc = vsc->next) {
            if (equal(tok, vsc->name)) return vsc;
        }
    }
    return NULL;
}

static Type *find_typedef(Token *tok) {
    if (tok->kind == TK_IDENT) {
        VarScope *vsc = find_var(tok);
        if (vsc) return vsc->type_def;
    }
    return NULL;
}

static Type *find_tag(Token *tok) {
    for (Scope *sc = scope; sc; sc = sc->next) {
        for (TagScope *tsc = sc->tsc; tsc; tsc = tsc->next) {
            if (equal(tok, tsc->name)) return tsc->ty;
        }
    }
    return NULL;
}
static Node *new_node(NodeKind kind, Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->tok  = tok;
    return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok) {
    Node *node = new_node(kind, tok);
    node->lhs  = lhs;
    node->rhs  = rhs;
    return node;
}

static Node *new_unary(NodeKind kind, Node *expr, Token *tok) {
    Node *node = new_node(kind, tok);
    node->lhs  = expr;
    return node;
}

Node *new_cast(Node *expr, Type *ty) {
    add_type(expr);

    Node *node = new_unary(ND_CAST, expr, expr->tok);
    node->ty   = copy_type(ty);
    return node;
}

static Node *new_num(long val, Token *tok) {
    Node *node = new_node(ND_NUM, tok);
    node->val  = val;
    return node;
}

static Node *new_long(long val, Token *tok) {
    Node *node = new_node(ND_NUM, tok);
    node->val  = val;
    node->ty   = ty_long;
    return node;
}

static Node *new_var_node(Obj *var, Token *tok) {
    Node *node = new_node(ND_VAR, tok);
    node->var  = var;
    return node;
}

static Obj *new_var(char *name, Type *ty) {
    Obj *var              = calloc(1, sizeof(Obj));
    var->name             = name;
    var->ty               = ty;
    push_scope(name)->var = var;
    return var;
}

static Obj *new_lvar(char *name, Type *ty) {
    Obj *var      = new_var(name, ty);
    var->is_local = true;
    var->next     = locals;
    locals        = var;
    return var;
}

static Obj *new_gvar(char *name, Type *ty) {
    Obj *var  = new_var(name, ty);
    var->next = globals;
    globals   = var;
    return var;
}

static char *new_unique_name(void) {
    static int id = 0;
    return format(".L.str.%d", id++);
}

static Obj *new_string_literal(char *p, Type *ty) {
    Obj *var       = new_gvar(new_unique_name(), ty);
    var->init_data = p;
    return var;
}

static long get_number(Token *tok) {
    if (tok->kind != TK_NUM) error_tok(tok, "expected an identifier");
    return tok->val;
}

static char *get_ident(Token *tok) {
    if (tok->kind != TK_IDENT) error_tok(tok, "expected an identifier");
    return strndup(tok->loc, tok->len);
}

// struct-members = (declspec declarator ("," declarator)* ";")*
static Member *struct_members(Token **rest, Token *tok) {
    Member head = {};
    Member *cur = &head;

    while (!equal(tok, "}")) {
        int i        = 0;
        Type *basety = declspec(&tok, tok, NULL);
        while (!equal(tok, ";")) {
            if (i++ > 0) { tok = skip(tok, ","); }

            Type *ty       = declarator(&tok, tok, basety);
            Member *member = calloc(1, sizeof(Member));
            member->name   = ty->name;
            member->ty     = ty;
            member->offset = ty->size;
            cur = cur->next = member;
        }
        tok = skip(tok, ";");
    }
    *rest = skip(tok, "}");
    return head.next;
}

// struct-decl = ident? "{" struct-members
static Type *struct_decl(Token **rest, Token *tok) {
    Token *tag = NULL;
    if (tok->kind == TK_IDENT) {
        tag = tok;
        tok = tok->next;
    }

    if (tag && !equal(tok, "{")) {
        Type *ty = find_tag(tag);
        if (!ty) error_tok(tok, "unknown struct type");
        *rest = tok;
        return ty;
    }

    tok = skip(tok, "{");

    Type *ty    = calloc(1, sizeof(Type));
    ty->kind    = TY_STRUCT;
    ty->members = struct_members(rest, tok);
    ty->align   = 1;

    // member offset
    int offset = 0;
    for (Member *m = ty->members; m; m = m->next) {
        offset    = align_to(offset, m->ty->align);
        m->offset = offset;
        offset += m->ty->size;

        if (ty->align < m->ty->align) ty->align = m->ty->align;
    }

    ty->size = align_to(offset, ty->align);

    if (tag) push_tag_scope(get_ident(tag), ty);

    return ty;
}

static Member *get_member(Member *members, Token *tok) {
    for (Member *m = members; m; m = m->next) {
        if (tok->len == m->name->len &&
            !strcmp(get_ident(tok), get_ident(m->name)))
            return m;
    }
    error_tok(tok, "no such member");
}

static Node *struct_ref(Node *node, Token *tok) {
    add_type(node);
    if (node->ty->kind != TY_STRUCT) error_tok(tok, "not a struct");

    Node *n   = new_unary(ND_MEMBER, node, tok);
    n->member = get_member(node->ty->members, tok);
    return n;
}

// declspec = ("void" | "_Bool" | "char" | "int" | "long" | "struct" struct-decl
// | typedef-name)+
static Type *declspec(Token **rest, Token *tok, VarAttr *attr) {
    enum {
        VOID  = 1 << 0,
        BOOL  = 1 << 2,
        CHAR  = 1 << 4,
        SHORT = 1 << 6,
        INT   = 1 << 8,
        LONG  = 1 << 10,
        OTHER = 1 << 12,
    };

    int counter = 0;

    while (is_typename(tok)) {
        if (equal(tok, "typedef")) {
            if (!attr)
                error_tok(
                    tok,
                    "storage class specifier is not allowed in this context");
            attr->is_typedef = true;
            tok              = tok->next;
            continue;
        }

        Type *ty = find_typedef(tok);

        if (ty) {
            *rest = tok->next;
            return ty;
        }

        if (equal(tok, "struct")) return struct_decl(rest, tok->next);
        if (equal(tok, "void"))
            counter += VOID;
        else if (equal(tok, "_Bool"))
            counter += BOOL;
        else if (equal(tok, "char"))
            counter += CHAR;
        else if (equal(tok, "short"))
            counter += SHORT;
        else if (equal(tok, "int"))
            counter += INT;
        else if (equal(tok, "long"))
            counter += LONG;
        tok = tok->next;
    }

    *rest = tok;
    switch (counter) {
        case VOID: return ty_void;
        case BOOL: return ty_bool;
        case CHAR: return ty_char;
        case SHORT:
        case SHORT + INT: return ty_short;
        case INT: return ty_int;
        case LONG:
        case LONG + LONG:
        case LONG + INT:
        case LONG + LONG + INT: return ty_long;
        default: {
            if (attr->is_typedef) return ty_int;
            error_tok(tok, "invalid type");
        }
    }
}

// func-params = (param ("," param)*)*)? ")"
// param       = declspec declarator
static Type *func_params(Token **rest, Token *tok, Type *ty) {
    Type head = {};
    Type *cur = &head;

    while (!equal(tok, ")")) {
        if (cur != &head) tok = skip(tok, ",");
        Type *basety = declspec(&tok, tok, NULL);
        Type *ty     = declarator(&tok, tok, basety);
        cur = cur->next = copy_type(ty);
    }

    ty         = func_type(ty);
    ty->params = head.next;
    *rest      = tok->next;
    return ty;
}

// type-suffix = "(" func-params
//             | "[" num "]" type-suffix
//             | ε
static Type *type_suffix(Token **rest, Token *tok, Type *ty) {
    if (equal(tok, "(")) return func_params(rest, tok->next, ty);

    if (equal(tok, "[")) {
        int sz = get_number(tok->next);
        tok    = skip(tok->next->next, "]");
        ty     = type_suffix(rest, tok, ty);
        return array_of(ty, sz);
    }

    *rest = tok;
    return ty;
}

// abstract-declarator = "*"* type-suffix
static Type *abstract_declarator(Token **rest, Token *tok, Type *ty) {
    while (consume(&tok, tok, "*"))
        ty = pointer_to(ty);

    ty       = type_suffix(rest, tok, ty);
    ty->name = tok;
    return ty;
}

// declarator = "*"* ident type-suffix
static Type *declarator(Token **rest, Token *tok, Type *ty) {
    while (consume(&tok, tok, "*"))
        ty = pointer_to(ty);

    if (tok->kind != TK_IDENT) error_tok(tok, "expected a variable name");

    ty       = type_suffix(rest, tok->next, ty);
    ty->name = tok;
    return ty;
}

// declaration = declspec (declarator ("=" expr)? ("," declarator ("="
// expr)?)*)? ";"
static Node *declaration(Token **rest, Token *tok, Type *basety) {
    Node head = {};
    Node *cur = &head;
    int i     = 0;

    while (!equal(tok, ";")) {
        if (i++ > 0) tok = skip(tok, ",");

        Type *ty = declarator(&tok, tok, basety);

        if (ty->kind == TY_VOID) error_tok(tok, "variable declared 'void'");

        Obj *obj = new_lvar(get_ident(ty->name), ty);

        if (!equal(tok, "=")) { continue; }

        Node *lhs  = new_var_node(obj, ty->name);
        Node *rhs  = assign(&tok, tok->next);
        Node *node = new_binary(ND_ASSIGN, lhs, rhs, tok);
        cur = cur->next = new_unary(ND_EXPR_STMT, node, tok);
    }

    Node *node = new_node(ND_BLOCK, tok);
    node->body = head.next;
    *rest      = tok->next;
    return node;
}

// stmt = "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "for" "(" expr-stmt expr? ";" expr? ")" stmt
//      | "while" "(" expr ")" stmt
//      | "{" compound-stmt
//      | expr-stmt
static Node *stmt(Token **rest, Token *tok) {
    if (equal(tok, "return")) {
        Node *node = new_node(ND_RETURN, tok);
        Node *exp  = expr(&tok, tok->next);
        add_type(exp);
        node->lhs = new_cast(exp, current_fn->ty->return_ty);
        *rest     = skip(tok, ";");
        return node;
    }

    if (equal(tok, "if")) {
        Node *node = new_node(ND_IF, tok);

        // cond
        tok        = skip(tok->next, "(");
        node->cond = expr(&tok, tok);
        tok        = skip(tok, ")");

        // then
        node->then = stmt(&tok, tok);

        // else
        if (equal(tok, "else")) node->els = stmt(&tok, tok->next);

        *rest = tok;
        return node;
    }

    if (equal(tok, "for")) {
        Node *node = new_node(ND_FOR, tok);
        tok        = skip(tok->next, "(");

        // init
        node->init = expr_stmt(&tok, tok);

        // cond
        if (!equal(tok, ";")) node->cond = expr(&tok, tok);
        tok = skip(tok, ";");

        // inc
        if (!equal(tok, ")")) node->inc = expr(&tok, tok);
        tok = skip(tok, ")");

        // then
        node->then = stmt(rest, tok);
        return node;
    }

    if (equal(tok, "while")) {
        Node *node = new_node(ND_FOR, tok);
        tok        = skip(tok->next, "(");
        node->cond = expr(&tok, tok);
        tok        = skip(tok, ")");
        node->then = stmt(rest, tok);
        return node;
    }

    if (equal(tok, "{")) return compound_stmt(rest, tok->next);

    return expr_stmt(rest, tok);
}

static bool is_typename(Token *tok) {
    char *kw[] = {
        "char", "int", "struct", "void", "long", "short", "typedef", "_Bool",
    };

    for (int i = 0; i < sizeof(kw) / sizeof(kw[0]); i++)
        if (equal(tok, kw[i])) return true;

    if (find_typedef(tok)) return true;
    return false;
}

// compound-stmt = (declaration | stmt)* "}"
static Node *compound_stmt(Token **rest, Token *tok) {
    enter_scope();
    Node *node = new_node(ND_BLOCK, tok);
    Node head  = {};
    Node *cur  = &head;

    while (!equal(tok, "}")) {
        if (is_typename(tok)) {
            VarAttr attr = {};
            Type *basety = declspec(&tok, tok, &attr);

            if (attr.is_typedef) {
                tok = parse_typedef(tok, basety);
                continue;
            }

            cur = cur->next = declaration(&tok, tok, basety);
        } else {
            cur = cur->next = stmt(&tok, tok);
        }

        add_type(cur);
    }

    node->body = head.next;
    *rest      = tok->next;
    leave_scope();
    return node;
}

// expr-stmt = expr? ";"
static Node *expr_stmt(Token **rest, Token *tok) {
    if (equal(tok, ";")) {
        *rest = tok->next;
        return new_node(ND_BLOCK, tok);
    }

    Node *node = new_node(ND_EXPR_STMT, tok);
    node->lhs  = expr(&tok, tok);
    *rest      = skip(tok, ";");
    return node;
}

// expr = assign ("," expr)?
static Node *expr(Token **rest, Token *tok) {
    Node *lhs = assign(&tok, tok);
    if (!equal(tok, ",")) {
        *rest = tok;
        return lhs;
    }

    return new_binary(ND_COMMA, lhs, expr(rest, tok->next), tok);
}

// assign = equality ("=" assign)?
static Node *assign(Token **rest, Token *tok) {
    Node *node = equality(&tok, tok);

    if (equal(tok, "="))
        return new_binary(ND_ASSIGN, node, assign(rest, tok->next), tok);

    *rest = tok;
    return node;
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality(Token **rest, Token *tok) {
    Node *node = relational(&tok, tok);

    for (;;) {
        Token *start = tok;

        if (equal(tok, "==")) {
            node = new_binary(ND_EQ, node, relational(&tok, tok->next), start);
            continue;
        }

        if (equal(tok, "!=")) {
            node = new_binary(ND_NE, node, relational(&tok, tok->next), start);
            continue;
        }

        *rest = tok;
        return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational(Token **rest, Token *tok) {
    Node *node = add(&tok, tok);

    for (;;) {
        Token *start = tok;

        if (equal(tok, "<")) {
            node = new_binary(ND_LT, node, add(&tok, tok->next), start);
            continue;
        }

        if (equal(tok, "<=")) {
            node = new_binary(ND_LE, node, add(&tok, tok->next), start);
            continue;
        }

        if (equal(tok, ">")) {
            node = new_binary(ND_LT, add(&tok, tok->next), node, start);
            continue;
        }

        if (equal(tok, ">=")) {
            node = new_binary(ND_LE, add(&tok, tok->next), node, start);
            continue;
        }

        *rest = tok;
        return node;
    }
}

static Node *new_add(Node *lhs, Node *rhs, Token *tok) {
    add_type(lhs);
    add_type(rhs);

    // num + num
    if (is_integer(lhs->ty) && is_integer(rhs->ty))
        return new_binary(ND_ADD, lhs, rhs, tok);

    // ptr + ptr
    if (lhs->ty->base && rhs->ty->base) error_tok(tok, "invalid operands");

    // Canonicalize `num + ptr` to `ptr + num`
    if (!lhs->ty->base && rhs->ty->base) {
        Node *tmp = lhs;
        lhs       = rhs;
        rhs       = tmp;
    }

    // ptr + num
    rhs = new_binary(ND_MUL, rhs, new_long(lhs->ty->base->size, tok), tok);
    return new_binary(ND_ADD, lhs, rhs, tok);
}

static Node *new_sub(Node *lhs, Node *rhs, Token *tok) {
    add_type(lhs);
    add_type(rhs);

    // num - num
    if (is_integer(lhs->ty) && is_integer(rhs->ty))
        return new_binary(ND_SUB, lhs, rhs, tok);

    // ptr - num
    if (lhs->ty->base && is_integer(rhs->ty)) {
        rhs = new_binary(ND_MUL, rhs, new_long(lhs->ty->base->size, tok), tok);
        return new_binary(ND_SUB, lhs, rhs, tok);
        // add_type(rhs);
        // Node *node = new_binary(ND_SUB, lhs, rhs, tok);
        // node->ty = lhs->ty;
        // return node;
    }

    // num - ptr
    if (is_integer(lhs->ty) && lhs->ty->base)
        error_tok(tok, "invalid operands");

    // ptr - ptr, which returns how many elements are between the two.
    Node *node = new_binary(ND_SUB, lhs, rhs, tok);
    node->ty   = ty_int;
    return new_binary(ND_DIV, node, new_num(lhs->ty->base->size, tok), tok);
}

// add = mul ("+" mul | "-" mul)*
static Node *add(Token **rest, Token *tok) {
    Node *node = mul(&tok, tok);

    for (;;) {
        Token *start = tok;

        if (equal(tok, "+")) {
            node = new_add(node, mul(&tok, tok->next), start);
            continue;
        }

        if (equal(tok, "-")) {
            node = new_sub(node, mul(&tok, tok->next), start);
            continue;
        }

        *rest = tok;
        return node;
    }
}

// mul = cast ("*" cast | "/" unary)*
static Node *mul(Token **rest, Token *tok) {
    Node *node = cast(&tok, tok);

    for (;;) {
        Token *start = tok;

        if (equal(tok, "*")) {
            node = new_binary(ND_MUL, node, cast(&tok, tok->next), start);
            continue;
        }
        if (equal(tok, "/")) {
            node = new_binary(ND_DIV, node, cast(&tok, tok->next), start);
            continue;
        }

        *rest = tok;
        return node;
    }
}

static Type *typename(Token **rest, Token *tok) {
    Type *ty = declspec(&tok, tok, NULL);
    return abstract_declarator(rest, tok, ty);
}

// cast = "(" typename ")" cast | unary
static Node *cast(Token **rest, Token *tok) {
    if (equal(tok, "(") && is_typename(tok->next)) {
        Token *start = tok;
        Type *ty     = typename(&tok, tok->next);
        tok          = skip(tok, ")");
        Node *node   = new_cast(cast(rest, tok), ty);
        node->tok    = start;
        return node;
    }
    return unary(rest, tok);
}

// unary = ("+" | "-" | "&" | "*") cast
//       | "sizeof" cast
//       | "sizeof" "( typename )"
//       | postfix
static Node *unary(Token **rest, Token *tok) {
    if (equal(tok, "+")) return cast(rest, tok->next);

    if (equal(tok, "-")) return new_unary(ND_NEG, cast(rest, tok->next), tok);

    if (equal(tok, "&")) return new_unary(ND_ADDR, cast(rest, tok->next), tok);

    if (equal(tok, "*")) return new_unary(ND_DEREF, cast(rest, tok->next), tok);

    if (equal(tok, "sizeof")) {
        if (equal(tok->next, "(") && is_typename(tok->next->next)) {
            Type *ty = typename(&tok, tok->next->next);
            *rest    = skip(tok, ")");
            return new_num(ty->size, tok);
        } else {
            Node *node = cast(rest, tok->next);
            add_type(node);
            return new_num(node->ty->size, tok);
        }
    }

    return postfix(rest, tok);
}

// postfix = primary ("[" expr "]" | "." ident | "->" ident)*
static Node *postfix(Token **rest, Token *tok) {
    Node *node = primary(&tok, tok);

    for (;;) {
        // Array
        if (equal(tok, "[")) {
            // x[y] is short for *(x+y)
            Token *start = tok;
            Node *idx    = expr(&tok, tok->next);
            tok          = skip(tok, "]");
            node = new_unary(ND_DEREF, new_add(node, idx, start), start);
            continue;
        }

        // Struct
        if (equal(tok, ".")) {
            node = struct_ref(node, tok->next);
            tok  = tok->next->next;
            continue;
        }

        if (equal(tok, "->")) {
            // x->a == (*x).a
            node = new_unary(ND_DEREF, node, tok);
            node = struct_ref(node, tok->next);
            tok  = tok->next->next;
            continue;
        }

        *rest = tok;
        return node;
    }
}

// funcall = ident "(" (assign ("," assign)*)? ")"
static Node *funcall(Token **rest, Token *tok) {
    Token *ident = tok;
    tok          = tok->next->next; // eat ident and "(".

    VarScope *sc = find_var(ident);

    if (!sc) error_tok(ident, "implicit declaration of a function");
    if (!sc->var || sc->var->ty->kind != TY_FUNC)
        error_tok(ident, "not a function");

    Type *ty       = sc->var->ty;
    Type *param_ty = ty->params;

    Node head = {};
    Node *cur = &head;

    while (!equal(tok, ")")) {
        if (cur != &head) tok = skip(tok, ",");

        Node *arg = assign(&tok, tok);
        add_type(arg);

        if (param_ty) {
            if (param_ty->kind == TY_STRUCT)
                error_tok(arg->tok, "passing struct is not supported yet");
            arg      = new_cast(arg, param_ty);
            param_ty = param_ty->next;
        }

        cur = cur->next = arg;
    }

    *rest = skip(tok, ")");

    Node *node     = new_node(ND_FUNCALL, ident);
    node->funcname = strndup(ident->loc, ident->len);
    node->func_ty  = ty;
    node->ty       = ty->return_ty;
    node->args     = head.next;
    return node;
}

// primary = "(" expr ")" | funcall | ident func-args? | str | num
//         | "(" "{" stmt+ "}" ")"
static Node *primary(Token **rest, Token *tok) {
    if (equal(tok, "(")) {
        if (equal(tok->next, "{")) {
            // GNU statement expression.
            Node *node = new_node(ND_STMT_EXPR, tok);
            node->body = compound_stmt(&tok, tok->next->next)->body;
            *rest      = skip(tok, ")");
            return node;
        } else {
            Node *node = expr(&tok, tok->next);
            *rest      = skip(tok, ")");
            return node;
        }
    }

    if (tok->kind == TK_IDENT) {
        // Function call
        if (equal(tok->next, "(")) return funcall(rest, tok);

        // Variable
        Obj *var = find_var(tok)->var;
        if (!var) error_tok(tok, "undefined variable");
        *rest = tok->next;
        return new_var_node(var, tok);
    }
    if (tok->kind == TK_STR) {
        int len  = strlen(tok->str) + 1; // terminating '\0'
        Obj *var = new_string_literal(tok->str, array_of(ty_char, len));

        *rest = tok->next;
        return new_var_node(var, tok);
    }

    if (tok->kind == TK_NUM) {
        Node *node = new_num(get_number(tok), tok);
        *rest      = tok->next;
        return node;
    }

    error_tok(tok, "expected an expression");
}

static void create_param_lvars(Type *param) {
    if (param) {
        create_param_lvars(param->next);
        new_lvar(get_ident(param->name), param);
    }
}

static Token *funcdef(Token *tok, Type *basety) {
    Type *ty = declarator(&tok, tok, basety);

    Obj *fn            = new_gvar(get_ident(ty->name), ty);
    fn->is_function    = true;
    fn->is_declaration = consume(&tok, tok, ";");

    if (fn->is_declaration) return tok;

    current_fn = fn;

    locals = NULL;

    enter_scope();
    create_param_lvars(ty->params);
    fn->params = locals;

    tok        = skip(tok, "{");
    fn->body   = compound_stmt(&tok, tok);
    fn->locals = locals;
    leave_scope();
    return tok;
}

static Token *global_variable(Token *tok, Type *basety) {
    bool first = true;

    while (!consume(&tok, tok, ";")) {
        if (!first) tok = skip(tok, ",");
        first = false;

        Type *ty = declarator(&tok, tok, basety);
        new_gvar(get_ident(ty->name), ty);
    }

    return tok;
}

// Lookahead tokens and returns true if a given token is a start
// of a function definition or declaration.
static bool is_function(Token *tok) {
    if (equal(tok, ";")) return false;

    Type dummy = {};
    Type *ty   = declarator(&tok, tok, &dummy);
    return ty->kind == TY_FUNC;
}

static Token *parse_typedef(Token *tok, Type *basety) {
    bool first = true;

    while (!consume(&tok, tok, ";")) {
        if (!first) tok = skip(tok, ",");
        first = false;

        Type *ty = declarator(&tok, tok, basety);
        push_scope(get_ident(ty->name))->type_def = ty;
    }

    return tok;
}

// program = (typedef | function-definition | global-variable)*
Obj *parse(Token *tok) {
    enter_scope();
    globals = NULL;

    while (tok->kind != TK_EOF) {
        VarAttr attr = {};
        Type *basety = declspec(&tok, tok, &attr);

        // Typedef
        if (attr.is_typedef) {
            tok = parse_typedef(tok, basety);
            continue;
        }

        // Function
        if (is_function(tok)) {
            tok = funcdef(tok, basety);
            continue;
        }

        // Global variable
        tok = global_variable(tok, basety);
    }

    leave_scope();
    return globals;
}
