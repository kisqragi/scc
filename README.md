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
declarator    = "*"* ident type-suffix
type-suffix   = ("(" func-params? ")")?
func-params   = param ("," param)*
param         = declspec declarator
declspec      = "int"
expr-stmt     = expr? ";"
expr          = assign
assign        = equality ("=" assign)?
equality      = relational ("==" relational | "!=" relational)*
relational    = add ("<" add | "<=" add | ">" add | ">=" add)*
add           = mul ("+" mul | "-" mul)*
mul           = unary ("*" unary | "/" unary)*
unary         = ("+" | "-" | "&" | "*") unary
funcall       = ident "(" (assign ("," assign)*)? ")"
primary       = "(" expr ")" | funcall | ident | num
```
