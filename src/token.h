#pragma once

#include <string>

// X-macro: single source of truth for all token types.
// Add a new token here and the enum + tokenTypeName stay in sync automatically.
#define TOKEN_TYPES(X)  \
    /* Keywords */       \
    X(KW_FN)            \
    X(KW_LET)           \
    X(KW_MUT)           \
    X(KW_IF)            \
    X(KW_ELSE)          \
    X(KW_WHILE)         \
    X(KW_RETURN)        \
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
