#pragma once

#include "token.h"
#include <istream>

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
