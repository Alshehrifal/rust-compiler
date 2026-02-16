# Rust Compiler -- Project Instructions

## Build and Test

```bash
cmake -B build
cmake --build build
cd build && ctest --output-on-failure
```

## Run

```bash
./build/rustc <filename.rs>
```

## Architecture

- **Lexer**: `src/token.h`, `src/lexer.h`, `src/lexer.cc` -- tokenizes Rust source
- **CLI**: `src/main.cc` -- reads file, runs lexer, prints tokens
- **Tests**: `tests/lexer_test.cc` -- Google Test suite

## Conventions

- C++17 standard
- Immutable token values (Token is a value type)
- Error tokens for invalid input (no exceptions for bad source)
- Google Test for all testing
- TDD workflow: tests first, then implementation

## Specification

See `docs/lexer.md` for the full token type list and grammar.
See `docs/architecture.md` for project overview.
