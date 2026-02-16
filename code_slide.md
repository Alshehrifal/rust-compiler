# Rust Compiler -- Code Slide

## 1. Entry Point: `main.cc`

The program starts here. It reads input (from a file or stdin) and feeds it to the lexer.

```cpp
#include "lexer.h"
#include <fstream>
#include <iostream>

int main(int argc, char* argv[]) {
    std::istream* input = &std::cin;
    std::ifstream file;

    if (argc > 1) {
        file.open(argv[1]);
        if (!file.is_open()) {
            std::cerr << "Error: cannot open file '" << argv[1] << "'" << std::endl;
            return 1;
        }
        input = &file;
    }

    Lexer lexer(*input);
    while (lexer.hasNext()) {
        Token tok = lexer.nextToken();
        std::cout << "[" << tokenTypeName(tok.type);
        if (!tok.lexeme.empty()) {
            std::cout << ":" << tok.lexeme;
        }
        std::cout << "]" << std::endl;
    }

    return 0;
}
```

**What it does:**

- Defaults to reading from `stdin`. If a filename is passed (`./build/rustc foo.rs`), it opens that file instead.
- Creates a `Lexer` with the input stream.
- Loops: calls `nextToken()` repeatedly, printing each token as `[TYPE:lexeme]`.
- Stops when `hasNext()` returns false (EOF reached).

**Example output** for `fn main() { let x = 42; }`:

```
[KW_FN:fn]
[IDENT:main]
[LPAREN:(]
[RPAREN:)]
[LBRACE:{]
[KW_LET:let]
[IDENT:x]
[EQUALS:=]
[NUMBER:42]
[SEMICOLON:;]
[RBRACE:}]
[EOF_TOK]
```

---

## 2. Token Definition: `token.h`

### TokenType Enum

Defines every kind of token the lexer can produce:

```cpp
enum class TokenType {
    // Keywords
    KW_FN, KW_LET, KW_MUT, KW_IF, KW_ELSE, KW_WHILE, KW_RETURN, KW_STRUCT,

    // Literals
    IDENT, NUMBER, STRING,

    // Operators
    PLUS, MINUS, STAR, SLASH, EQUALS, EQEQ, BANGEQ,
    LT, GT, LTEQ, GTEQ, ARROW,

    // Punctuation
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    SEMICOLON, COLON, COMMA,

    // Special
    EOF_TOK, ERROR
};
```

### Token Struct

A simple value type with four fields:

```cpp
struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};
```

| Field    | Type          | Purpose                                      |
|----------|---------------|----------------------------------------------|
| `type`   | `TokenType`   | What kind of token (keyword, operator, etc.)  |
| `lexeme` | `std::string` | The raw text that was matched                 |
| `line`   | `int`         | Source line number (1-based)                  |
| `column` | `int`         | Source column number (1-based)                |

**Why `line` and `column`?** They exist for error reporting. Right now the compiler only prints tokens, but once a parser or type checker is added, error messages need to point the user to the exact location in their source file. They are 1-based because that matches what every text editor displays.

### tokenTypeName Function

```cpp
inline const char* tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::KW_FN:      return "KW_FN";
        case TokenType::KW_LET:     return "KW_LET";
        // ... one case per token type ...
        case TokenType::ERROR:     return "ERROR";
    }
    return "UNKNOWN";
}
```

**Why `const char*` instead of `std::string`?** The function only returns string literals -- hardcoded text that already lives in static, read-only memory. Returning `std::string` would allocate heap memory on every call to copy those same characters. Since the data is already there, `const char*` avoids that unnecessary cost. Both `std::cout <<` and string concatenation accept `const char*`, so there is no compatibility issue.

---

## 3. Lexer Interface: `lexer.h`

```cpp
class Lexer {
public:
    explicit Lexer(std::istream& input);

    bool hasNext() const;
    Token nextToken();

private:
    std::istream& input_;
    int line_;
    int column_;
    bool eof_;

    char peek();
    char advance();
    void skipWhitespace();
    Token makeToken(TokenType type, const std::string& lexeme, int startLine, int startCol);
    Token readIdentOrKeyword();
    Token readNumber();
    Token readString();
};
```

**Public API:**
- `Lexer(input)` -- construct with an input stream
- `hasNext()` -- returns `true` until EOF is reached
- `nextToken()` -- returns the next token from the input

**Private state:**
- `input_` -- reference to the character stream
- `line_`, `column_` -- current position in the source (for error reporting)
- `eof_` -- set to `true` once end-of-file is reached

---

## 4. Lexer Implementation: `lexer.cc`

### 4.1 Keyword Map

```cpp
static const std::unordered_map<std::string, TokenType> keywords = {
    {"fn",     TokenType::KW_FN},
    {"let",    TokenType::KW_LET},
    {"mut",    TokenType::KW_MUT},
    {"if",     TokenType::KW_IF},
    {"else",   TokenType::KW_ELSE},
    {"while",  TokenType::KW_WHILE},
    {"return", TokenType::KW_RETURN},
    {"struct", TokenType::KW_STRUCT},
};
```

A hash map used to distinguish keywords from identifiers. After reading a word, the lexer looks it up here.

### 4.2 Core Character Operations

**`peek()`** -- looks at the next character without consuming it. Returns `'\0'` at end-of-stream.

```cpp
char Lexer::peek() {
    int ch = input_.peek();
    if (ch == std::char_traits<char>::eof()) {
        return '\0';
    }
    return static_cast<char>(ch);
}
```

**`advance()`** -- consumes and returns the next character. Updates line/column tracking (newlines reset column to 1 and increment line).

```cpp
char Lexer::advance() {
    int ch = input_.get();
    if (ch == std::char_traits<char>::eof()) {
        return '\0';
    }
    char c = static_cast<char>(ch);
    if (c == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    return c;
}
```

These two primitives power the entire lexer.

### 4.3 Skip Whitespace

```cpp
void Lexer::skipWhitespace() {
    while (true) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else {
            break;
        }
    }
}
```

Consumes spaces, tabs, carriage returns, and newlines. They are not meaningful tokens.

### 4.4 `nextToken()` -- Main Dispatch

This is the heart of the lexer. Each call:

1. Skips whitespace
2. Records the current line/column
3. Peeks at the next character and dispatches:

| Character seen       | Action                                       |
|----------------------|----------------------------------------------|
| `'\0'` (EOF)         | Sets `eof_ = true`, returns `EOF_TOK`        |
| Letter or `_`        | Calls `readIdentOrKeyword()`                 |
| Digit                | Calls `readNumber()`                         |
| `"`                  | Calls `readString()`                         |
| Anything else        | Consumes it, matches operator or punctuation |

### 4.5 Identifier and Keyword Recognition

```cpp
Token Lexer::readIdentOrKeyword() {
    int startLine = line_;
    int startCol = column_;
    std::string word;
    while (true) {
        char c = peek();
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '_') {
            word += advance();
        } else {
            break;
        }
    }
    auto it = keywords.find(word);
    if (it != keywords.end()) {
        return makeToken(it->second, word, startLine, startCol);
    }
    return makeToken(TokenType::IDENT, word, startLine, startCol);
}
```

Accumulates characters matching `[a-zA-Z0-9_]`, then checks the keyword map. If the word is a keyword like `fn`, it returns `KW_FN`. Otherwise it returns `IDENT`.

### 4.6 Number Recognition

```cpp
Token Lexer::readNumber() {
    int startLine = line_;
    int startCol = column_;
    std::string num;
    while (true) {
        char c = peek();
        if ((c >= '0' && c <= '9') || c == '_') {
            num += advance();
        } else {
            break;
        }
    }
    return makeToken(TokenType::NUMBER, num, startLine, startCol);
}
```

Reads digits and underscores, supporting Rust's numeric separator syntax (e.g. `1_000_000`).

### 4.7 String Recognition

```cpp
Token Lexer::readString() {
    int startLine = line_;
    int startCol = column_;
    advance(); // consume opening "
    std::string value;
    while (true) {
        char c = peek();
        if (c == '\0') {
            return makeToken(TokenType::ERROR, value, startLine, startCol);
        }
        if (c == '"') {
            advance(); // consume closing "
            break;
        }
        if (c == '\\') {
            advance();
            char next = peek();
            if (next == '\0') {
                return makeToken(TokenType::ERROR, value, startLine, startCol);
            }
            advance();
            switch (next) {
                case 'n':  value += '\n'; break;
                case 't':  value += '\t'; break;
                case '\\': value += '\\'; break;
                case '"':  value += '"';  break;
                default:   value += next; break;
            }
        } else {
            value += advance();
        }
    }
    return makeToken(TokenType::STRING, value, startLine, startCol);
}
```

Reads between `"` quotes. Handles escape sequences (`\n`, `\t`, `\\`, `\"`). Returns an `ERROR` token if EOF is hit before the closing quote.

### 4.8 Multi-Character Operator Disambiguation

Some operators need a second character to resolve:

| First char | If next is... | Result       | Otherwise |
|------------|---------------|--------------|-----------|
| `-`        | `>`           | `ARROW` (->)  | `MINUS`   |
| `=`        | `=`           | `EQEQ` (==)   | `EQUALS`  |
| `!`        | `=`           | `BANGEQ` (!=) | `ERROR`   |
| `<`        | `=`           | `LTEQ` (<=)   | `LT`      |
| `>`        | `=`           | `GTEQ` (>=)   | `GT`      |

A standalone `!` produces an `ERROR` token since the lexer does not recognize it on its own.

### 4.9 Error Handling

The lexer never throws exceptions. Instead it returns `ERROR` tokens for:
- Unrecognized characters (e.g. `@`, `#`)
- Standalone `!` (only `!=` is valid)
- Unterminated strings (EOF before closing `"`)

---

## 5. Data Flow Summary

```
Source file (.rs)
    |
    v
main.cc: open file or stdin
    |
    v
Lexer(input stream)
    |
    v
Loop: nextToken()
    |-- skipWhitespace()
    |-- peek() to classify the next character
    |-- readIdentOrKeyword() / readNumber() / readString() / operator switch
    |-- return Token { type, lexeme, line, column }
    |
    v
Print: [TYPE:lexeme]
    |
    v
EOF_TOK -> exit
```

The compiler currently implements only the lexing (tokenization) phase. The next phases -- parsing into an AST, type checking, and code generation -- are not yet implemented.
