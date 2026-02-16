# Lexer Specification

## Token Types

The lexer produces tokens of the following types:

### Keywords

| TokenType | Lexeme |
|-----------|--------|
| KW_FN | `fn` |
| KW_LET | `let` |
| KW_MUT | `mut` |
| KW_IF | `if` |
| KW_ELSE | `else` |
| KW_WHILE | `while` |
| KW_RETURN | `return` |
| KW_STRUCT | `struct` |

### Literals

| TokenType | Description | Examples |
|-----------|-------------|----------|
| IDENT | Identifier | `foo`, `_bar`, `myVar` |
| NUMBER | Integer literal | `42`, `0`, `3_141_592` |
| STRING | String literal | `"hello"`, `"line\n"` |

### Operators

| TokenType | Lexeme |
|-----------|--------|
| PLUS | `+` |
| MINUS | `-` |
| STAR | `*` |
| SLASH | `/` |
| EQUALS | `=` |
| EQEQ | `==` |
| BANGEQ | `!=` |
| LT | `<` |
| GT | `>` |
| LTEQ | `<=` |
| GTEQ | `>=` |
| ARROW | `->` |

### Punctuation

| TokenType | Lexeme |
|-----------|--------|
| LPAREN | `(` |
| RPAREN | `)` |
| LBRACE | `{` |
| RBRACE | `}` |
| LBRACKET | `[` |
| RBRACKET | `]` |
| SEMICOLON | `;` |
| COLON | `:` |
| COMMA | `,` |

### Special

| TokenType | Description |
|-----------|-------------|
| EOF_TOK | End of input |
| ERROR | Unrecognized character |

## Lexical Grammar (EBNF)

```
token       = keyword | ident | number | string | operator | punct | error
keyword     = "fn" | "let" | "mut" | "if" | "else" | "while" | "return" | "struct"
ident       = (letter | "_") { letter | digit | "_" }
number      = digit { digit | "_" }
string      = '"' { string_char } '"'
string_char = escape_seq | <any char except '"' and '\'>
escape_seq  = '\' ( 'n' | 't' | '\\' | '"' )
operator    = "==" | "!=" | "<=" | ">=" | "->" | "+" | "-" | "*" | "/" | "=" | "<" | ">"
punct       = "(" | ")" | "{" | "}" | "[" | "]" | ";" | ":" | ","
whitespace  = " " | "\t" | "\r" | "\n"
letter      = "a".."z" | "A".."Z"
digit       = "0".."9"
error       = <any unrecognized character>
```

Whitespace is skipped between tokens but used to track line/column positions.

## Edge Cases

### Keyword vs Identifier
- `fn` is KW_FN, but `fn_name` is IDENT (longest match)
- `let` is KW_LET, but `letter` is IDENT
- `return` is KW_RETURN, but `returning` is IDENT

### Multi-Character Operators
- `==` is a single EQEQ token, not two EQUALS tokens
- `!=` is a single BANGEQ token
- `->` is a single ARROW token, not MINUS then GT
- `<=` is a single LTEQ token
- `>=` is a single GTEQ token
- A lone `!` (not followed by `=`) is an ERROR token

### Numbers
- Underscores are allowed within digits: `1_000_000` has lexeme `1_000_000`
- Leading underscore makes it an identifier: `_42` is IDENT
- Underscore-only `_` is IDENT

### String Literals
- Escape sequences: `\n` (newline), `\t` (tab), `\\` (backslash), `\"` (quote)
- Empty string `""` is valid
- Unterminated string (EOF before closing quote) produces ERROR token

## Tokenization Examples

### Example 1: Variable declaration
Input: `let x = 42;`
```
[KW_LET:"let"] [IDENT:"x"] [EQUALS:"="] [NUMBER:"42"] [SEMICOLON:";"] [EOF_TOK]
```

### Example 2: Function declaration
Input: `fn main() {}`
```
[KW_FN:"fn"] [IDENT:"main"] [LPAREN:"("] [RPAREN:")"] [LBRACE:"{"] [RBRACE:"}"] [EOF_TOK]
```

### Example 3: Function with body
Input:
```rust
fn add(a: i32, b: i32) -> i32 {
    return a + b;
}
```
```
[KW_FN:"fn"] [IDENT:"add"] [LPAREN:"("] [IDENT:"a"] [COLON:":"] [IDENT:"i32"]
[COMMA:","] [IDENT:"b"] [COLON:":"] [IDENT:"i32"] [RPAREN:")"] [ARROW:"->"]
[IDENT:"i32"] [LBRACE:"{"] [KW_RETURN:"return"] [IDENT:"a"] [PLUS:"+"]
[IDENT:"b"] [SEMICOLON:";"] [RBRACE:"}"] [EOF_TOK]
```
