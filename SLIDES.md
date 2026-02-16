# Rust Subset Compiler

### A Rust-to-Machine-Code Compiler Written in C++17

---

# Slide 1: Project Overview

- A **Rust subset compiler** built from scratch in **C++17**
- Currently in **Phase 1** -- Lexer (complete)
- Follows **Test-Driven Development** (TDD)
- Follows **Documentation-Driven Development** (DDD)
- Built with **CMake** on **macOS ARM64** (Apple Silicon)

---

# Slide 2: Project Structure

```
rust-compiler/
|-- src/
|   |-- token.h          # Token types & data structure
|   |-- lexer.h          # Lexer class declaration
|   |-- lexer.cc         # Lexer implementation
|   |-- main.cc          # CLI entry point
|-- tests/
|   |-- lexer_test.cc    # 54 Google Test cases
|-- docs/
|   |-- architecture.md  # Roadmap & design
|   |-- lexer.md         # Lexer specification
|-- build/
|   |-- rustc            # Compiled binary (102 KB)
|   |-- lexer_test       # Test binary (1.5 MB)
|-- CMakeLists.txt       # Build configuration
|-- example.rs           # Sample Rust input
|-- README.md
```

---

# Slide 3: Compiler Phases (Roadmap)

```
Source Code (.rs)
      |
      v
+--------------+
| Phase 1      |   <-- COMPLETE
| Lexer        |   Tokenizes source into token stream
+--------------+
      |
      v
+--------------+
| Phase 2      |   <-- NEXT
| Parser       |   Builds Abstract Syntax Tree (AST)
+--------------+
      |
      v
+--------------+
| Phase 3      |
| Semantic     |   Type checking, name resolution,
| Analysis     |   borrow checking (subset)
+--------------+
      |
      v
+--------------+
| Phase 4      |
| Code Gen     |   Target: LLVM IR or x86-64 assembly
+--------------+
      |
      v
  Executable
```

---

# Slide 4: The Token -- Core Data Structure

```cpp
struct Token {
    TokenType type;      // What kind of token
    std::string lexeme;  // Original text from source
    int line;            // Line number (1-indexed)
    int column;          // Column number (1-indexed)
};
```

- Immutable value type -- never mutated after creation
- Carries source location for error reporting

---

# Slide 5: Token Types (35 Total)

| Category         | Tokens                                                  | Count |
|------------------|---------------------------------------------------------|-------|
| **Keywords**     | `fn` `let` `mut` `if` `else` `while` `return` `struct` | 8     |
| **Literals**     | Identifiers, Numbers, Strings                           | 3     |
| **Arithmetic**   | `+`  `-`  `*`  `/`                                      | 4     |
| **Assignment**   | `=`                                                     | 1     |
| **Comparison**   | `==`  `!=`  `<`  `>`  `<=`  `>=`                        | 6     |
| **Arrow**        | `->`                                                    | 1     |
| **Punctuation**  | `(` `)` `{` `}` `[` `]` `;` `:` `,`                    | 9     |
| **Special**      | `EOF_TOK`  `ERROR`                                      | 2     |
| **Total**        |                                                         | **35**|

---

# Slide 6: Lexer Class Design

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

    char peek();              // Lookahead without consuming
    char advance();           // Consume and return next char
    void skipWhitespace();    // Skip spaces, tabs, newlines
    Token readIdentOrKeyword();
    Token readNumber();
    Token readString();
    Token makeToken(...);
};
```

- Reads from any `std::istream` (file or stdin)
- Single-character lookahead for multi-char operators
- Tracks line and column for every token

---

# Slide 7: Tokenization Algorithm

```
Start
  |
  v
Skip whitespace (spaces, tabs, \r, \n)
  |
  v
Read first character --> classify:
  |
  |-- Letter / _    --> readIdentOrKeyword()
  |                      then check keyword hash map
  |
  |-- Digit          --> readNumber()
  |                      (supports underscore separators: 1_000)
  |
  |-- Double quote   --> readString()
  |                      (supports escape sequences)
  |
  |-- Operator char  --> check for multi-char:
  |                      == != <= >= ->
  |
  |-- Punctuation    --> single-char token
  |
  |-- Unknown        --> ERROR token
  |
  v
Return Token with (type, lexeme, line, column)
```

---

# Slide 8: Supported Rust Features

### Keywords
`fn` | `let` | `mut` | `if` | `else` | `while` | `return` | `struct`

### Literals
- **Identifiers:** `foo`, `_bar`, `myVar_123`
- **Integers:** `42`, `0`, `1_000_000` (underscore separators)
- **Strings:** `"hello"`, `"line\n"` with escape sequences (`\n`, `\t`, `\\`, `\"`)

### Operators
- Arithmetic: `+` `-` `*` `/`
- Comparison: `==` `!=` `<` `>` `<=` `>=`
- Assignment: `=`
- Return type arrow: `->`

### Punctuation
`(` `)` `{` `}` `[` `]` `;` `:` `,`

---

# Slide 9: Edge Cases Handled

| Edge Case                    | Behavior                               |
|------------------------------|----------------------------------------|
| `fn_name` vs `fn`           | `fn_name` = identifier (longest match) |
| `==` vs `= =`               | `==` is one EQEQ token                |
| `->` vs `- >`               | `->` is one ARROW token               |
| `1_000`                      | Valid number with underscore separator |
| `_42`                        | Identifier, not a number               |
| `""`                         | Valid empty string                     |
| Unterminated `"hello`        | ERROR token                            |
| Unknown char `@` `#`        | ERROR token, continues scanning        |
| Lone `!`                     | ERROR token                            |
| Empty input                  | Immediate EOF token                    |

---

# Slide 10: Example Input & Output

### Input (`example.rs`):
```rust
fn add(a: i32, b: i32) -> i32 {
    return a + b;
}

fn main() {
    let x = 42;
    let mut y = 0;
    if x >= y {
        y = x + 1;
    }
}
```

### Output:
```
[KW_FN:fn] [IDENT:add] [LPAREN:(] [IDENT:a] [COLON::]
[IDENT:i32] [COMMA:,] [IDENT:b] [COLON::] [IDENT:i32]
[RPAREN:)] [ARROW:->] [IDENT:i32] [LBRACE:{]
[KW_RETURN:return] [IDENT:a] [PLUS:+] [IDENT:b]
[SEMICOLON:;] [RBRACE:}] ...
```

---

# Slide 11: Build System

### Dependencies
- **CMake 3.14+** -- Build system generator
- **C++17** -- Language standard (required)
- **Google Test v1.14.0** -- Auto-fetched via CMake FetchContent

### Build Commands
```bash
cmake -B build                          # Configure
cmake --build build                     # Compile
cd build && ctest --output-on-failure   # Run tests
./build/rustc example.rs                # Run compiler
```

### Usage
```bash
./build/rustc example.rs          # Tokenize from file
cat example.rs | ./build/rustc    # Tokenize from stdin
```

---

# Slide 12: Testing -- 54 Test Cases

### Test Categories

| Category              | Tests | Examples                              |
|-----------------------|-------|---------------------------------------|
| Keywords              | 8     | `fn`, `let`, `mut`, `if`, `else`...   |
| Identifiers           | 4     | `foo`, `_bar`, `myVar_123`, `fn_name` |
| Numbers               | 3     | `42`, `0`, `3_141_592`                |
| Strings               | 4     | `"hello"`, escapes, empty, multi-word |
| Arithmetic Operators  | 4     | `+`, `-`, `*`, `/`                    |
| Assignment            | 1     | `=`                                   |
| Comparison Operators  | 6     | `==`, `!=`, `<`, `>`, `<=`, `>=`      |
| Arrow Operator        | 1     | `->`                                  |
| Punctuation           | 9     | `(`, `)`, `{`, `}`, `[`, `]`...      |
| Whitespace Handling   | 2     | Skipping, line tracking               |
| Combined Tokens       | 3     | `let x = 42;`, `fn main() {}`        |
| Edge Cases            | 9     | Empty input, errors, ambiguity        |

**Test-to-code ratio: 56% tests, 44% implementation**

---

# Slide 13: Code Metrics

| Metric                  | Value                |
|-------------------------|----------------------|
| Total source lines      | 824                  |
| Implementation lines    | 363 (44%)            |
| Test lines              | 461 (56%)            |
| Token types             | 35                   |
| Supported keywords      | 8                    |
| Test cases              | 54                   |
| Test pass rate          | 100%                 |
| Compiled binary         | 102 KB               |
| Test binary             | 1.5 MB (incl. gtest) |
| Platform                | macOS ARM64          |

---

# Slide 14: Design Principles

1. **Incremental Development**
   - Each phase is self-contained and testable
   - Phase 1 works independently before Phase 2 begins

2. **Test-Driven Development (TDD)**
   - Tests written before implementation
   - Red -> Green -> Refactor cycle

3. **Documentation-Driven Development (DDD)**
   - Specs in `docs/` written before code
   - `lexer.md` defines grammar, `architecture.md` defines roadmap

4. **Error Resilience**
   - Invalid input produces ERROR tokens, never crashes
   - No exceptions for bad source code

5. **Immutability**
   - Token is a value type -- never mutated after creation
   - Lexer state is fully encapsulated

---

# Slide 15: What's Next -- Phase 2 (Parser)

### Goals
- Consume token stream from the Lexer
- Build an **Abstract Syntax Tree** (AST)
- Handle the Rust grammar subset

### Expected AST Nodes
- `FnDecl` -- function declarations
- `LetStmt` -- variable bindings
- `IfExpr` -- conditional expressions
- `WhileExpr` -- loop expressions
- `BinaryExpr` -- arithmetic and comparison
- `ReturnExpr` -- return statements
- `StructDecl` -- struct definitions

### Not Yet Supported (Future Work)
- Comments (`//` and `/* */`)
- Boolean literals (`true`, `false`)
- Pattern matching, enums, traits
- Lifetime annotations
- Macros

---

# Slide 16: Summary

```
+------------------------------------------+
|         Rust Subset Compiler             |
|         Written in C++17                 |
+------------------------------------------+
|                                          |
|  Phase 1: Lexer           [COMPLETE]     |
|  - 35 token types                        |
|  - 8 Rust keywords                       |
|  - 54 passing tests                      |
|  - Full error resilience                 |
|  - Position tracking                     |
|                                          |
|  Phase 2: Parser           [NEXT]        |
|  Phase 3: Semantic Analysis [PLANNED]    |
|  Phase 4: Code Generation   [PLANNED]    |
|                                          |
+------------------------------------------+
|  824 lines | 100% tests pass | C++17    |
+------------------------------------------+
```
