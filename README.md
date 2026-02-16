# Rust Compiler

A compiler for a subset of the Rust programming language, written in C++17.

## Current Status

**Phase 1: Lexer** -- Tokenizes Rust source code into a stream of tokens.

Supported tokens: keywords (`fn`, `let`, `mut`, `if`, `else`, `while`, `return`,
`struct`), identifiers, integer literals, string literals with escape sequences,
arithmetic and comparison operators, arrow (`->`), and punctuation.

## Build

Requires CMake 3.14+ and a C++17 compiler.

```bash
cmake -B build
cmake --build build
```

## Test

```bash
cd build && ctest --output-on-failure
```

## Usage

```bash
./build/rustc example.rs
```

Prints each token in `[TYPE:lexeme]` format, one per line.

## Project Structure

```
src/token.h       Token types and struct
src/lexer.h       Lexer class declaration
src/lexer.cc      Lexer implementation
src/main.cc       CLI entry point
tests/            Google Test suite
docs/             Specifications and architecture
```
