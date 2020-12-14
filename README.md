# scc is small C compiler

```
program    = stmt*
stmt       = "return" expr ";"
           | expr-stmt
expr-stmt  = expr ";"
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-") unary | primary
primary    = "(" expr ")" | ident | num
```
