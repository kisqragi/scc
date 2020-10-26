#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node Node;
typedef struct Type Type;


//
// tokenize.c
//

typedef enum {
    TK_IDENT,
    TK_PUNCT,
    TK_KEYWORD,
    TK_NUM,
    TK_EOF,
} TokenKind;

// Token type
typedef struct Token Token;
struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *loc;
    int len;
};

void error(char *fmt, ...);
void verror_at(char *loc, char *fmt, va_list ap);
void error_at(char *loc, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
bool equal(Token *tok, char *op);
Token *skip(Token *tok, char *s);
bool consume(Token **rest, Token *tok, char *str);
Token *tokenize(char *input);

//
// parse.c
//
//

// Local variable
typedef struct Obj Obj;
struct Obj {
    Obj *next;
    char *name; // Variable name
    Type *ty;   // ty
    int offset; // Offset from RBP
};

// Function
typedef struct Function Function;
struct Function {
    Function *next;
    char *name;
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
    ND_ADDR,        // unary &
    ND_DEREF,       // unary *
    ND_RETURN,
    ND_IF,
    ND_FOR,         // "for" or "while"
    ND_BLOCK,       // {...}
    ND_FUNCALL,     // Function call
    ND_EXPR_STMT,
    ND_VAR,
    ND_NUM,
} NodeKind;

// AST node type
struct Node {
    NodeKind kind;
    Node *next;
    Type *ty;   // Type, e.g. int or pointer to int
    Token *tok; // Representative token

    Node *lhs;
    Node *rhs;

    // Block
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
    int val;    // Used if kind == ND_NUM
};

Function *parse(Token *tok);

//
// codegen.c
//

void codegen(Function *prog);

//
// type.c
//

typedef enum  {
    TY_INT,
    TY_PTR,
    TY_FUNC,
    TY_ARRAY,
} TypeKind;

struct Type {
    TypeKind kind;

    // Pointer
    int size;   // sizeof() value

    Type *base;

    // Declaration
    Token *name;

    // Array
    int array_len;

    // Function type
    Type *return_ty;
    Type *params;
    Type *next;
};

extern Type *ty_int;

bool is_integer(Type *ty);
bool is_pointer(Type *ty);
Type *copy_type(Type *ty);
Type *pointer_to(Type *base);
Type *array_of(Type *base, int size);
Type *func_type(Type *return_ty);
void add_type(Node *node);
