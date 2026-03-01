#pragma once

#include "ast.h"
#include "token.h"
#include <stdexcept>
#include <vector>

class ParseError : public std::runtime_error {
public:
    Token token;

    ParseError(const std::string& message, const Token& token)
        : std::runtime_error(
              "Parse error at line " + std::to_string(token.line) +
              ", column " + std::to_string(token.column) + ": " + message),
          token(token) {}
};

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    std::unique_ptr<ProgramNode> parse();

private:
    const std::vector<Token>& tokens_;
    size_t current_;

    // Navigation
    const Token& peek() const;
    const Token& previous() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& expect(TokenType type, const std::string& message);
    bool isAtEnd() const;

    // Declarations
    std::unique_ptr<FuncDeclNode> parseFuncDecl();
    std::vector<Param> parseParamList();
    std::string parseType();

    // Statements
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<BlockNode> parseBlock();
    std::unique_ptr<LetStmtNode> parseLetStmt();
    std::unique_ptr<ReturnStmtNode> parseReturnStmt();
    std::unique_ptr<IfStmtNode> parseIfStmt();
    std::unique_ptr<WhileStmtNode> parseWhileStmt();
    std::unique_ptr<ForStmtNode> parseForStmt();
    std::unique_ptr<ExprStmtNode> parseExprStmt();

    // Expressions (precedence climbing)
    std::unique_ptr<Expr> parseExpr();
    std::unique_ptr<Expr> parseAssignment();
    std::unique_ptr<Expr> parseBinExpr(int minPrec);
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePostfix();
    std::unique_ptr<Expr> parsePrimary();
    std::vector<std::unique_ptr<Expr>> parseArgList();
};
