# Rust Compiler

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![CMake](https://img.shields.io/badge/CMake-3.14%2B-064F8C.svg)
![Tests](https://img.shields.io/badge/tests-54%20passing-brightgreen.svg)
![Coverage](https://img.shields.io/badge/test--to--code-56%25-yellow.svg)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Linux-lightgrey.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

A compiler for a subset of the Rust programming language, written in C++17.

## Current Status

**Phase 1: Lexer** -- Complete

| Phase | Status | Description |
|-------|--------|-------------|
| 1. Lexer | Done | Tokenizes Rust source into token stream |
| 2. Parser | Planned | Builds Abstract Syntax Tree (AST) |
| 3. Semantic Analysis | Planned | Type checking, name resolution |
| 4. Code Generation | Planned | LLVM IR or x86-64 assembly |

## Prerequisites

- **C++17** compatible compiler (GCC 7+, Clang 5+, Apple Clang 10+)
- **CMake 3.14+**
- **Git** (for fetching Google Test)

### macOS

```bash
xcode-select --install            # Apple Clang
brew install cmake                # CMake
```

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install build-essential cmake git
```

### Fedora

```bash
sudo dnf install gcc-c++ cmake git
```

## Build

```bash
git clone https://github.com/Alshehrifal/rust-compiler.git
cd rust-compiler
cmake -B build
cmake --build build
```

Google Test is fetched automatically via CMake FetchContent during configuration.

## Run Tests

```bash
cd build && ctest --output-on-failure
```

Or run the test binary directly for verbose output:

```bash
./build/lexer_test
```

All 54 tests should pass:

```
54/54 Test ... Passed
100% tests passed, 0 tests failed out of 54
```

## Usage

Tokenize a Rust source file:

```bash
./build/rustc example.rs
```

Or pipe from stdin:

```bash
echo 'let x = 42;' | ./build/rustc
```

Output format -- one token per line:

```
[KW_LET:let]
[IDENT:x]
[EQUALS:=]
[NUMBER:42]
[SEMICOLON:;]
[EOF_TOK]
```

## Supported Rust Features

| Category | Tokens |
|----------|--------|
| Keywords | `fn` `let` `mut` `if` `else` `while` `return` `struct` |
| Literals | Identifiers, integers (with `_` separators), strings (with escapes) |
| Operators | `+` `-` `*` `/` `=` `==` `!=` `<` `>` `<=` `>=` `->` |
| Punctuation | `(` `)` `{` `}` `[` `]` `;` `:` `,` |

## Project Structure

```
rust-compiler/
|-- src/
|   |-- token.h          # Token types (X-macro) and struct
|   |-- lexer.h          # Lexer class declaration
|   |-- lexer.cc         # Lexer implementation (212 lines)
|   |-- main.cc          # CLI entry point
|-- tests/
|   |-- lexer_test.cc    # 54 Google Test cases
|-- docs/
|   |-- architecture.md  # Roadmap and design
|   |-- lexer.md         # Lexer specification and grammar
|-- CMakeLists.txt       # Build configuration
|-- example.rs           # Sample Rust input
```

## Design Principles

- **X-Macro Pattern** -- Token enum and name function generated from a single list in `token.h`
- **Test-Driven Development** -- Tests written before implementation, 56% test-to-code ratio
- **Error Resilience** -- Invalid input produces `ERROR` tokens, never crashes
- **Immutability** -- `Token` is a value type, never mutated after creation
- **Zero Dependencies** -- Only Google Test for testing (auto-fetched)
