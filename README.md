# scc is small C compiler

```
program    = stmt*
stmt       = expr-stmt
expr-stmt  = expr ";"
expr       = equality
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-") unary | primary
primary    = "(" expr ")" | num
```
