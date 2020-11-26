# scc is small C compiler

```
program             = (typedef | function-definition | global-variable)*
stmt                = "return" expr ";"
                    | "if" "(" expr ")" stmt ("else" stmt)?
                    | "for" "(" expr-stmt expr? ";" expr? ")" stmt
                    | "while" "(" expr ")" stmt
                    | "{" compound-stmt
                    | expr-stmt
compound-stmt       = (declaration | stmt)* "}"
declaration         = declspec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
type-suffix         = "(" func-params
                    | "[" num "]" type-suffix
                    | ε
func-params         = (param ("," param)*)*)? ")"
param               = declspec declarator
declspec            = ("char" | "int" | "long" | "struct" struct-decl | typedef-name)+
struct-decl         = ident? "{" struct-members
struct-members      = (declspec declarator ("," declarator)* ";")*
declarator          = "*"* ident type-suffix
expr-stmt           = expr? ";"
expr                = assign
assign              = equality ("=" assign)?
equality            = relational ("==" relational | "!=" relational)*
relational          = add ("<" add | "<=" add | ">" add | ">=" add)*
add                 = mul ("+" mul | "-" mul)*
mul                 = unary ("*" unary | "/" unary)*
abstract-declarator = "*"* type-suffix
unary               = ("+" | "-" | "&" | "*") unary
                    | "sizeof" "( typename )"
                    | "sizeof" unary
                    | postfix
postfix             = primary ("[" expr "]" | "." ident | "->" ident)*
funcall             = ident "(" (assign ("," assign)*)? ")"
primary             = "(" expr ")" | funcall | ident | num
                    | "(" "{" stmt+ "}" ")"
```
