#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct Node Node;
typedef struct Type Type;
typedef struct Member Member;

//
// tokenize.c
//

typedef enum {
    TK_IDENT,
    TK_PUNCT,
    TK_KEYWORD,
    TK_STR,
    TK_NUM,
    TK_EOF,
} TokenKind;

// Token type
typedef struct Token Token;
struct Token {
    TokenKind kind;
    Token *next;
    long val;
    char *loc;
    int len;
    char *str;      // String literal contents including terminating '\0'

    int line_no;    // Line number
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
bool equal(Token *tok, char *op);
Token *skip(Token *tok, char *s);
bool consume(Token **rest, Token *tok, char *str);
Token *tokenize_file(char *path);

//
// parse.c
//
//

// Variable or function
typedef struct Obj Obj;
struct Obj {
    Obj *next;
    char *name;     // Variable name
    Type *ty;       // Type
    bool is_local;  // local or global/function

    // Local variable (Offset from RBP)
    int offset;

    // Global variable or function
    bool is_function;
    bool is_declaration;

    // Global variable
    char *init_data;

    // Function
    Obj *params;
    Node *body;
    Obj *locals;
    int stack_size;
};

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NEG,         // unary -
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
    ND_ASSIGN,
    ND_COMMA,
    ND_MEMBER,
    ND_ADDR,        // unary &
    ND_DEREF,       // unary *
    ND_RETURN,
    ND_IF,
    ND_FOR,         // "for" or "while"
    ND_BLOCK,       // {...}
    ND_FUNCALL,     // Function call
    ND_EXPR_STMT,
    ND_STMT_EXPR,
    ND_VAR,
    ND_NUM,
    ND_CAST,
} NodeKind;

// AST node type
struct Node {
    NodeKind kind;
    Node *next;
    Type *ty;   // Type, e.g. int or pointer to int
    Token *tok; // Representative token

    Node *lhs;
    Node *rhs;

    // Block or statement expression
    Node *body;

    // Function call
    char *funcname;
    Node *args;

    // "if" or "for" statement
    Node *cond;
    Node *then;
    Node *els;
    Node *init;
    Node *inc;

    Obj *var;   // Used if kind == ND_VAR
    long val;    // Used if kind == ND_NUM

    // Struct
    Member *member;
};

Obj *parse(Token *tok);
Node *new_cast(Node *expr, Type *ty);

struct Member {
    Member *next;
    Type *ty;
    Token *name;
    int offset;
};

//
// codegen.c
//

void codegen(Obj *prog, FILE *out);
int align_to(int n, int align);

//
// strings.c
//

char *format(char *fmt, ...);

//
// type.c
//

typedef enum  {
    TY_VOID,
    TY_BOOL,
    TY_CHAR,
    TY_INT,
    TY_SHORT,
    TY_LONG,
    TY_PTR,
    TY_FUNC,
    TY_ARRAY,
    TY_STRUCT,
} TypeKind;

struct Type {
    TypeKind kind;

    // Pointer
    int size;   // sizeof() value

    // alignment
    int align;

    Type *base;

    // Declaration
    Token *name;

    // Array
    int array_len;

    // Function type
    Type *return_ty;
    Type *params;
    Type *next;

    // struct
    Member *members;
};

extern Type *ty_void;
extern Type *ty_bool;
extern Type *ty_char;
extern Type *ty_short;
extern Type *ty_int;
extern Type *ty_long;

bool is_integer(Type *ty);
bool is_pointer(Type *ty);
Type *copy_type(Type *ty);
Type *pointer_to(Type *base);
Type *array_of(Type *base, int size);
Type *func_type(Type *return_ty);
void add_type(Node *node);
