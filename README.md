# scc is small C compiler

```
program       = function-definition*
stmt          = "return" expr ";"
              | "if" "(" expr ")" stmt ("else" stmt)?
              | "for" "(" expr-stmt expr? ";" expr? ")" stmt
              | "while" "(" expr ")" stmt
              | "{" compound-stmt
              | expr-stmt
compound-stmt = (declaration | stmt)* "}"
declaration   = declspec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
type-suffix   = "(" func-params
              | "[" num "]" type-suffix
              | ε
func-params   = (param ("," param)*)*)? ")"
param         = declspec declarator
declspec      = "int"
declarator    = "*"* ident type-suffix
expr-stmt     = expr? ";"
expr          = assign
assign        = equality ("=" assign)?
equality      = relational ("==" relational | "!=" relational)*
relational    = add ("<" add | "<=" add | ">" add | ">=" add)*
add           = mul ("+" mul | "-" mul)*
mul           = unary ("*" unary | "/" unary)*
unary         = ("+" | "-" | "&" | "*") unary
              | "sizeof" unary
              | postfix
postfix       = primary ("[" expr "]")*
funcall       = ident "(" (assign ("," assign)*)? ")"
primary       = "(" expr ")" | funcall | ident | num
```
