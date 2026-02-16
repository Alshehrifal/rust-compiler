#include "lexer.h"
#include <unordered_map>

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
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        return readIdentOrKeyword();
    }

    // Numbers
    if (c >= '0' && c <= '9') {
        return readNumber();
    }

    // String literals
    if (c == '"') {
        return readString();
    }

    // Operators and punctuation
    advance();

    switch (c) {
        case '+': return makeToken(TokenType::PLUS,      "+", startLine, startCol);
        case '*': return makeToken(TokenType::STAR,      "*", startLine, startCol);
        case '/': return makeToken(TokenType::SLASH,     "/", startLine, startCol);
        case '(': return makeToken(TokenType::LPAREN,    "(", startLine, startCol);
        case ')': return makeToken(TokenType::RPAREN,    ")", startLine, startCol);
        case '{': return makeToken(TokenType::LBRACE,    "{", startLine, startCol);
        case '}': return makeToken(TokenType::RBRACE,    "}", startLine, startCol);
        case '[': return makeToken(TokenType::LBRACKET,  "[", startLine, startCol);
        case ']': return makeToken(TokenType::RBRACKET,  "]", startLine, startCol);
        case ';': return makeToken(TokenType::SEMICOLON, ";", startLine, startCol);
        case ':': return makeToken(TokenType::COLON,     ":", startLine, startCol);
        case ',': return makeToken(TokenType::COMMA,     ",", startLine, startCol);

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
