#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
Token *tokenize(char *input);

//
// parse.c
//
//

typedef struct Node Node;

// Local variable
typedef struct Obj Obj;
struct Obj {
    Obj *next;
    char *name; // Variable name
    int offset; // Offset from RBP
};

// Function
typedef struct Function Function;
struct Function {
    Node *body;
    Obj *locals;
    int stack_size;
};

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NEG, // unary -
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
    ND_ASSIGN,
    ND_RETURN,
    ND_IF,
    ND_BLOCK,   // {...}
    ND_EXPR_STMT,
    ND_VAR,
    ND_NUM,
} NodeKind;

// AST node type
struct Node {
    NodeKind kind;
    Node *next;
    Node *lhs;
    Node *rhs;

    // Block
    Node *body;

    // "if" statement
    Node *cond;
    Node *then;
    Node *els;

    Obj *var;   // Used if kind == ND_VAR
    int val;    // Used if kind == ND_NUM
};

Function *parse(Token *tok);

//
// codegen.c
//

void codegen(Function *prog);
