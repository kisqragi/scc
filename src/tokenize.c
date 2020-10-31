#include "scc.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

// Input string
static char *current_input;

// Reports an error and exit.
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Reports an error location and exit.
void verror_at(char *loc, char *fmt, va_list ap) {
    int pos = loc - current_input;
    fprintf(stderr, "%s\n", current_input);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    verror_at(loc, fmt, ap);
}

void error_tok(Token *tok, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    verror_at(tok->loc, fmt, ap);
}

// Consumes the current token if it matches `s`.
bool equal(Token *tok, char *op) {
    return memcmp(tok->loc, op, tok->len) == 0 && op[tok->len] == '\0';
}

bool consume(Token **rest, Token *tok, char *str) {
    if (equal(tok, str)) {
        *rest = tok->next;
        return true;
    }
    *rest = tok;
    return false;
}

// Ensure that the current token is `s`.
Token *skip(Token *tok, char *s) {
    if (!equal(tok, s))
        error_tok(tok, "expected '%s'", s);
    return tok->next;
}

// Ensure that the current token is TK_NUM.
static int get_number(Token *tok) {
    if (tok->kind != TK_NUM)
        error_tok(tok, "expected a number");
    return tok->val;
}

// Create a new token.
static Token *new_token(TokenKind kind, char *start, char *end) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->loc = start;
    tok->len = end - start;
    return tok;
}

static bool startswith(char *p, char *q) {
    return strncmp(p, q, strlen(q)) == 0;
}

// Returns true if c is valid as the first character of an identifier.
static bool is_ident_first(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

// Returns true if c is valid as a non-first character of an identifier.
static bool is_ident_nfirst(char c) {
    return is_ident_first(c) || ('0' <= c && c <= '9');
}

// Read a punctuator token from p and returns its length.
static int read_punct(char *p) {
    if (startswith(p, "==") || startswith(p, "!=") ||
        startswith(p, ">=") || startswith(p, "<=")) {
        return 2;
    }

    return ispunct(*p) ? 1 : 0;
}

static bool is_keyword(Token *tok) {
    static char *kw[] = {"return", "if", "else", "for", "while", "int", "sizeof", "char"};

    for (int i = 0; i < sizeof(kw) / sizeof(kw[0]); i++) {
        if (equal(tok, kw[i]))
            return true;
    }

    return false;
}

static void convert_keywords(Token *tok) {
    for (Token *t = tok; t; t = t->next) {
        if (is_keyword(t))
            t->kind = TK_KEYWORD;
    }
}

static bool is_octal_range(char *p) {
    return ('0' <= *p && *p <= '7');
}

static bool is_hex_range(char *p) {
    return ('0' <= *p && *p <= '7');
}

static char read_escaped_char(char *p, char **endptr) {
    // octal number
    if (is_octal_range(p)) {
        int n = *p++ - '0';
        if (is_octal_range(p)) {
            n = (n << 3) + (*p++ - '0');
            if (is_octal_range(p)) {
                n = (n << 3) + (*p++ - '0');
            }
        }
        *endptr = p;
        return n;
    }

    // hexadecimal number
    if (*p == 'x') {
        if (!isxdigit(*(++p)))
            error_at(p, "invalid hex escape sequence");
        return strtoul(p, endptr, 16);
    }

    *endptr = p + 1;
    switch (*p) {
        case 'a': return  '\a';
        case 'b': return  '\b';
        case 't': return  '\t';
        case 'n': return  '\n';
        case 'v': return  '\v';
        case 'f': return  '\f';
        case 'r': return  '\r';
        // '\e' -> GNU C extention.
        case 'e': return  27;
        default: return *p;
    }
}

static char *string_literal_end(char *p) {
    char *start = p;
    for (; *p != '"'; p++) {
        if (*p == '\n' || *p == '\0')
            error_at(start, "unclosed string literal");
        if (*p == '\\')
            p++;
    }
    return p;
}

static Token *read_string_literal(char *start) {
    char *end = string_literal_end(start + 1);
    char *buf = calloc(1, end - start);
    int len = 0;

    for (char *p = start + 1; p < end;) {
        if (*p == '\\') {
            buf[len++] = read_escaped_char(p + 1, &p);
        } else {
            buf[len++] = *p++;
        }
    }

    Token *tok = new_token(TK_STR, start, end+1);
    tok->ty = array_of(ty_char, len + 1);
    tok->str = buf;
    return tok;
}

// Tokenize `p` and returns new tokens.
Token *tokenize(char *p) {
    current_input = p;

    Token head = {};
    Token *cur = &head;

    while (*p) {
        // Skip whitespace characters.
        if (isspace(*p)) {
            p++;
            continue;
        }

        // Numeric literal
        if (isdigit(*p)) {
            cur = cur->next = new_token(TK_NUM, p, p);
            char *q = p;
            cur->val = strtoul(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        // String literal
        if (*p == '"') {
            cur = cur->next = read_string_literal(p);
            p += cur->len;
            continue;
        }

        // Identifier or Keyword
        if (is_ident_first(*p)) {
            char *start = p;
            do {
                p++;
            } while (is_ident_nfirst(*p));
            cur = cur->next = new_token(TK_IDENT, start, p);
            continue;
        }

        // Punctuator
        int punct_len = read_punct(p);
        if (punct_len) {
            cur = cur->next = new_token(TK_PUNCT, p, p + punct_len);
            p += cur->len;
            continue;
        }

        error_at(p, "invalid token");
    }

    cur = cur->next = new_token(TK_EOF, p, p);
    convert_keywords(head.next);
    return head.next;
}
