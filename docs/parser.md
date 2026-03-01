# Parser Specification

## Overview

The parser consumes the token stream produced by the lexer and builds an
abstract syntax tree (AST). It is a hand-written recursive descent parser
that supports a subset of Rust: functions, let bindings, control flow,
expressions with operator precedence, function calls, and array indexing.

## Grammar (EBNF)

```
program        = { function_decl } EOF
function_decl  = "fn" IDENT "(" param_list ")" [ "->" type ] block
param_list     = [ param { "," param } ]
param          = IDENT ":" type
type           = IDENT
block          = "{" { statement } "}"

statement      = let_stmt | return_stmt | if_stmt | while_stmt | for_stmt | expr_stmt
let_stmt       = "let" [ "mut" ] IDENT [ ":" type ] "=" expr ";"
return_stmt    = "return" [ expr ] ";"
if_stmt        = "if" expr block [ "else" ( if_stmt | block ) ]
while_stmt     = "while" expr block
for_stmt       = "for" IDENT "in" expr block
expr_stmt      = expr ";"

expr           = assignment
assignment     = equality [ "=" assignment ]
equality       = comparison { ( "==" | "!=" ) comparison }
comparison     = addition { ( "<" | ">" | "<=" | ">=" ) addition }
addition       = multiplication { ( "+" | "-" ) multiplication }
multiplication = unary { ( "*" | "/" ) unary }
unary          = [ "-" ] postfix
postfix        = primary { "(" arg_list ")" | "[" expr "]" }
primary        = NUMBER | STRING | IDENT | "(" expr ")"
arg_list       = [ expr { "," expr } ]
```

## Expression Precedence (lowest to highest)

| Level | Operators | Associativity |
|-------|-----------|---------------|
| 1 | `=` | Right |
| 2 | `==` `!=` | Left |
| 3 | `<` `>` `<=` `>=` | Left |
| 4 | `+` `-` | Left |
| 5 | `*` `/` | Left |
| 6 | `-` (prefix) | Right |
| 7 | `()` `[]` (postfix) | Left |

## AST Node Types

### Expressions

| Node | Fields | Description |
|------|--------|-------------|
| NumberLit | value (string) | Integer literal |
| StringLit | value (string) | String literal |
| Ident | name (string) | Identifier reference |
| UnaryExpr | op (TokenType), operand (Expr) | Prefix operator |
| BinaryExpr | op (TokenType), left (Expr), right (Expr) | Infix operator |
| AssignExpr | target (Expr), value (Expr) | Assignment (right-associative) |
| CallExpr | callee (Expr), args (Expr[]) | Function call |
| IndexExpr | object (Expr), index (Expr) | Array indexing |

### Statements

| Node | Fields | Description |
|------|--------|-------------|
| ExprStmt | expr (Expr) | Expression followed by semicolon |
| LetStmt | name, type (optional), init (Expr), mutable (bool) | Variable binding |
| ReturnStmt | value (Expr, optional) | Return from function |
| Block | statements (Stmt[]) | Brace-delimited statement list |
| IfStmt | condition (Expr), thenBlock (Block), elseBlock (Stmt, optional) | Conditional |
| WhileStmt | condition (Expr), body (Block) | While loop |
| ForStmt | variable (string), iterable (Expr), body (Block) | For-in loop |

### Declarations

| Node | Fields | Description |
|------|--------|-------------|
| Param | name (string), type (string) | Function parameter (value type) |
| FuncDecl | name, params (Param[]), returnType (optional), body (Block) | Function declaration |
| Program | functions (FuncDecl[]) | Top-level program |

## Error Handling

The parser throws `ParseError` (extends `std::runtime_error`) on syntax errors.
Each error carries the offending `Token` for line/column reporting.

Error message format: `"Parse error at line X, column Y: message"`

Common error conditions:
- Unexpected token (expected specific token type)
- Missing semicolon after statement
- Missing closing delimiter (`}`, `)`, `]`)
- Missing `in` keyword in for loop
- Unexpected end of input

## Design Decisions

- **AssignExpr is separate from BinaryExpr**: Assignment is right-associative
  and has different semantics (target must be an lvalue).
- **Block is a Stmt subclass**: This allows `else` branches to naturally hold
  either a block or an if-statement.
- **IfStmt::elseBlock is Stmt***: Enables `else if` chains without special
  else-if node types.
- **Ownership via unique_ptr**: All child nodes are owned via
  `std::unique_ptr<T>`. No raw owning pointers.
- **NodeKind enum via X-macro**: Matches the project convention established
  by the lexer's TOKEN_TYPES macro.
