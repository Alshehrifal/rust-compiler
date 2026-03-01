# Rust Compiler -- Architecture

## Overview

A compiler for a subset of the Rust programming language, implemented in C++17.
The project is built incrementally, starting with lexical analysis and expanding
toward parsing, semantic analysis, and code generation.

## Directory Structure

```
rust-compiler/
  docs/              Documentation (DDD -- docs first)
    architecture.md  This file
    lexer.md         Lexer specification and grammar
    parser.md        Parser specification, grammar, and AST design
  src/               Source code
    token.h          Token types and Token struct
    lexer.h          Lexer class declaration
    lexer.cc         Lexer implementation
    ast.h            AST node hierarchy (X-macro NodeKind enum)
    parser.h         Parser class declaration + ParseError
    parser.cc        Recursive descent parser implementation
    main.cc          CLI entry point (--parse flag for AST output)
  tests/             Test suite
    lexer_test.cc    Lexer unit/integration tests (Google Test)
    parser_test.cc   Parser unit/integration tests (Google Test)
  CMakeLists.txt     Build system
  CLAUDE.md          Project-specific AI guidance
  README.md          Project readme
```

## Build Instructions

```bash
cmake -B build
cmake --build build
cd build && ctest --output-on-failure
```

## Components

### Lexer (Phase 1 -- Complete)

Converts a stream of characters into a stream of tokens. Handles keywords,
identifiers, numeric literals, string literals, operators, and punctuation.
See `docs/lexer.md` for the full specification.

### Parser (Phase 2 -- Current)

Recursive descent parser that consumes the token stream and produces an AST.
Supports functions, let bindings, if/else, while/for loops, return statements,
expressions with operator precedence, function calls, and array indexing.
See `docs/parser.md` for the grammar and AST design.

### Semantic Analysis (Phase 3 -- Future)

Type checking, name resolution, borrow checking (subset).

### Code Generation (Phase 4 -- Future)

Target TBD (LLVM IR or direct x86-64 assembly).

## Design Principles

- **Incremental**: Each phase is self-contained and testable
- **TDD**: Tests are written before implementation
- **Immutability**: Token objects are value types; lexer state is encapsulated
- **Error resilience**: Invalid input produces error tokens, not crashes
