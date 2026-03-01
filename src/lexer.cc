#include "lexer.h"
#include <unordered_map>

// Generated from KEYWORD_TOKENS X-macro
static const std::unordered_map<std::string, TokenType> keywords = {
#define KEYWORD_ENTRY(name, str) {str, TokenType::name},
    KEYWORD_TOKENS(KEYWORD_ENTRY)
#undef KEYWORD_ENTRY
};

static inline bool isIdentStart(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static inline bool isIdentContinue(char c) {
    return isIdentStart(c) || (c >= '0' && c <= '9');
}

static inline bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

Lexer::Lexer(std::istream& input)
    : input_(input), line_(1), column_(1), eof_(false) {}

bool Lexer::hasNext() const {
    return !eof_;
}

char Lexer::peek() {
    int ch = input_.peek();
    if (ch == std::char_traits<char>::eof()) {
        return '\0';
    }
    return static_cast<char>(ch);
}

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

Token Lexer::makeToken(TokenType type, const std::string& lexeme, int startLine, int startCol) {
    return Token{type, lexeme, startLine, startCol};
}

Token Lexer::readIdentOrKeyword() {
    int startLine = line_;
    int startCol = column_;
    std::string word;
    while (isIdentContinue(peek())) {
        word += advance();
    }
    auto it = keywords.find(word);
    if (it != keywords.end()) {
        return makeToken(it->second, word, startLine, startCol);
    }
    return makeToken(TokenType::IDENT, word, startLine, startCol);
}

Token Lexer::readNumber() {
    int startLine = line_;
    int startCol = column_;
    std::string num;
    while (isDigit(peek()) || peek() == '_') {
        num += advance();
    }
    return makeToken(TokenType::NUMBER, num, startLine, startCol);
}

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
            advance(); // consume backslash
            char next = peek();
            if (next == '\0') {
                return makeToken(TokenType::ERROR, value, startLine, startCol);
            }
            advance();
            switch (next) {
#define ESCAPE_CASE(esc, actual) case esc: value += actual; break;
                ESCAPE_SEQUENCES(ESCAPE_CASE)
#undef ESCAPE_CASE
                default: value += next; break;
            }
        } else {
            value += advance();
        }
    }
    return makeToken(TokenType::STRING, value, startLine, startCol);
}

Token Lexer::nextToken() {
    skipWhitespace();

    int startLine = line_;
    int startCol = column_;
    char c = peek();

    if (c == '\0') {
        eof_ = true;
        return makeToken(TokenType::EOF_TOK, "", startLine, startCol);
    }

    // Identifiers and keywords
    if (isIdentStart(c)) {
        return readIdentOrKeyword();
    }

    // Numbers
    if (isDigit(c)) {
        return readNumber();
    }

    // String literals
    if (c == '"') {
        return readString();
    }

    // Operators and punctuation
    advance();

    switch (c) {
        // Generated from SINGLE_CHAR_TOKENS X-macro
#define CHAR_CASE(name, ch) \
        case ch: return makeToken(TokenType::name, std::string(1, ch), startLine, startCol);
        SINGLE_CHAR_TOKENS(CHAR_CASE)
#undef CHAR_CASE

        case '-':
            if (peek() == '>') {
                advance();
                return makeToken(TokenType::ARROW, "->", startLine, startCol);
            }
            return makeToken(TokenType::MINUS, "-", startLine, startCol);

        case '=':
            if (peek() == '=') {
                advance();
                return makeToken(TokenType::EQEQ, "==", startLine, startCol);
            }
            return makeToken(TokenType::EQUALS, "=", startLine, startCol);

        case '!':
            if (peek() == '=') {
                advance();
                return makeToken(TokenType::BANGEQ, "!=", startLine, startCol);
            }
            return makeToken(TokenType::ERROR, "!", startLine, startCol);

        case '<':
            if (peek() == '=') {
                advance();
                return makeToken(TokenType::LTEQ, "<=", startLine, startCol);
            }
            return makeToken(TokenType::LT, "<", startLine, startCol);

        case '>':
            if (peek() == '=') {
                advance();
                return makeToken(TokenType::GTEQ, ">=", startLine, startCol);
            }
            return makeToken(TokenType::GT, ">", startLine, startCol);

        default:
            return makeToken(TokenType::ERROR, std::string(1, c), startLine, startCol);
    }
}
