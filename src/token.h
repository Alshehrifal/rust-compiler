#pragma once

#include <string>

// ---- X-macros: single source of truth for token metadata ----
// Add a new token to TOKEN_TYPES for the enum. Add metadata to the
// specific macro (KEYWORD_TOKENS, SINGLE_CHAR_TOKENS, ESCAPE_SEQUENCES)
// so the lexer and tests stay in sync automatically.

// All token types (enum + name mapping): X(enum_name)
#define TOKEN_TYPES(X)  \
    /* Keywords */       \
    X(KW_FN)            \
    X(KW_LET)           \
    X(KW_MUT)           \
    X(KW_IF)            \
    X(KW_ELSE)          \
    X(KW_WHILE)         \
    X(KW_RETURN)        \
    X(KW_FOR)           \
    X(KW_IN)            \
    X(KW_STRUCT)        \
    /* Literals */       \
    X(IDENT)            \
    X(NUMBER)           \
    X(STRING)           \
    /* Operators */      \
    X(PLUS)             \
    X(MINUS)            \
    X(STAR)             \
    X(SLASH)            \
    X(EQUALS)           \
    X(EQEQ)            \
    X(BANGEQ)           \
    X(LT)              \
    X(GT)              \
    X(LTEQ)            \
    X(GTEQ)            \
    X(ARROW)           \
    /* Punctuation */    \
    X(LPAREN)           \
    X(RPAREN)           \
    X(LBRACE)           \
    X(RBRACE)           \
    X(LBRACKET)         \
    X(RBRACKET)         \
    X(SEMICOLON)        \
    X(COLON)            \
    X(COMMA)            \
    /* Special */        \
    X(EOF_TOK)          \
    X(ERROR)

// Keywords: X(enum_name, keyword_string)
// Used by lexer (keyword map) and tests (keyword test generation).
#define KEYWORD_TOKENS(X)    \
    X(KW_FN,     "fn")      \
    X(KW_LET,    "let")     \
    X(KW_MUT,    "mut")     \
    X(KW_IF,     "if")      \
    X(KW_ELSE,   "else")    \
    X(KW_WHILE,  "while")   \
    X(KW_RETURN, "return")  \
    X(KW_FOR,    "for")     \
    X(KW_IN,     "in")      \
    X(KW_STRUCT, "struct")

// Single-character tokens (no lookahead): X(enum_name, character)
// Used by lexer (switch cases) and tests (single-char test generation).
#define SINGLE_CHAR_TOKENS(X) \
    X(PLUS,      '+')        \
    X(STAR,      '*')        \
    X(SLASH,     '/')        \
    X(LPAREN,    '(')        \
    X(RPAREN,    ')')        \
    X(LBRACE,    '{')        \
    X(RBRACE,    '}')        \
    X(LBRACKET,  '[')        \
    X(RBRACKET,  ']')        \
    X(SEMICOLON, ';')        \
    X(COLON,     ':')        \
    X(COMMA,     ',')

// String escape sequences: X(escape_char, replacement_char)
// Used by lexer (readString escape handling).
#define ESCAPE_SEQUENCES(X) \
    X('n',  '\n')           \
    X('t',  '\t')           \
    X('\\', '\\')           \
    X('"',  '"')

enum class TokenType {
#define DEFINE_TOKEN(name) name,
    TOKEN_TYPES(DEFINE_TOKEN)
#undef DEFINE_TOKEN
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};

inline const char* tokenTypeName(TokenType t) {
    switch (t) {
#define TOKEN_CASE(name) case TokenType::name: return #name;
        TOKEN_TYPES(TOKEN_CASE)
#undef TOKEN_CASE
    }
    return "UNKNOWN";
}
