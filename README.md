# scc is small C compiler

```
program       = stmt*
stmt          = "return" expr ";"
              | "if" "(" expr ")" stmt ("else" stmt)?
              | "for" "(" expr-stmt expr? ";" expr? ")" stmt
              | "while" "(" expr ")" stmt
              | "{" compound-stmt
              | expr-stmt
compound-stmt = (declaration | stmt)* "}"
declaration   = typespec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
declarator    = "*"* ident
typespec      = "int"
expr-stmt     = expr? ";"
expr          = assign
assign        = equality ("=" assign)?
equality      = relational ("==" relational | "!=" relational)*
relational    = add ("<" add | "<=" add | ">" add | ">=" add)*
add           = mul ("+" mul | "-" mul)*
mul           = unary ("*" unary | "/" unary)*
unary         = ("+" | "-" | "&" | "*") unary
primary       = "(" expr ")" | ident args? | num
args          = "(" ")"
```
