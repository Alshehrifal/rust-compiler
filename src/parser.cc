#include "parser.h"

Parser::Parser(const std::vector<Token>& tokens)
    : tokens_(tokens), current_(0) {}

// ------------------------------------------------------------
// Navigation
// ------------------------------------------------------------

const Token& Parser::peek() const {
    if (tokens_.empty()) {
        static const Token eofToken{TokenType::EOF_TOK, "", 1, 1};
        return eofToken;
    }
    return tokens_[current_];
}

const Token& Parser::previous() const {
    if (current_ == 0) {
        return tokens_[0];
    }
    return tokens_[current_ - 1];
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        current_++;
    }
    return previous();
}

bool Parser::check(TokenType type) const {
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

const Token& Parser::expect(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }
    throw ParseError(message, peek());
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOK;
}

// ------------------------------------------------------------
// Declarations
// ------------------------------------------------------------

std::unique_ptr<ProgramNode> Parser::parse() {
    int line = peek().line;
    int col = peek().column;
    std::vector<std::unique_ptr<FuncDeclNode>> functions;

    while (!isAtEnd()) {
        functions.push_back(parseFuncDecl());
    }

    return std::make_unique<ProgramNode>(std::move(functions), line, col);
}

std::unique_ptr<FuncDeclNode> Parser::parseFuncDecl() {
    const Token& fnTok = expect(TokenType::KW_FN, "expected 'fn'");
    const Token& nameTok = expect(TokenType::IDENT, "expected function name");
    std::string name = nameTok.lexeme;

    expect(TokenType::LPAREN, "expected '(' after function name");
    auto params = parseParamList();
    expect(TokenType::RPAREN, "expected ')' after parameters");

    std::string returnType;
    if (match(TokenType::ARROW)) {
        returnType = parseType();
    }

    auto body = parseBlock();

    return std::make_unique<FuncDeclNode>(
        name, std::move(params), returnType, std::move(body),
        fnTok.line, fnTok.column);
}

std::vector<Param> Parser::parseParamList() {
    std::vector<Param> params;
    if (check(TokenType::RPAREN)) {
        return params;
    }

    do {
        const Token& nameTok = expect(TokenType::IDENT, "expected parameter name");
        expect(TokenType::COLON, "expected ':' after parameter name");
        std::string typeName = parseType();
        params.push_back(Param{nameTok.lexeme, typeName});
    } while (match(TokenType::COMMA));

    return params;
}

std::string Parser::parseType() {
    const Token& tok = expect(TokenType::IDENT, "expected type name");
    return tok.lexeme;
}

// ------------------------------------------------------------
// Statements
// ------------------------------------------------------------

// Statement keyword dispatch: X(token_type, parse_method)
#define STMT_DISPATCH(X)                \
    X(KW_LET,    parseLetStmt)          \
    X(KW_RETURN, parseReturnStmt)       \
    X(KW_IF,     parseIfStmt)           \
    X(KW_WHILE,  parseWhileStmt)        \
    X(KW_FOR,    parseForStmt)

std::unique_ptr<Stmt> Parser::parseStatement() {
#define CHECK_STMT(token, method) \
    if (check(TokenType::token)) return method();
    STMT_DISPATCH(CHECK_STMT)
#undef CHECK_STMT
    if (check(TokenType::LBRACE)) return parseBlock();
    return parseExprStmt();
}

std::unique_ptr<BlockNode> Parser::parseBlock() {
    const Token& brace = expect(TokenType::LBRACE, "expected '{'");
    std::vector<std::unique_ptr<Stmt>> statements;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        statements.push_back(parseStatement());
    }

    expect(TokenType::RBRACE, "expected '}'");
    return std::make_unique<BlockNode>(std::move(statements),
                                       brace.line, brace.column);
}

std::unique_ptr<LetStmtNode> Parser::parseLetStmt() {
    const Token& letTok = advance(); // consume 'let'
    bool isMutable = match(TokenType::KW_MUT);

    const Token& nameTok = expect(TokenType::IDENT, "expected variable name");

    std::string typeName;
    if (match(TokenType::COLON)) {
        typeName = parseType();
    }

    expect(TokenType::EQUALS, "expected '=' in let binding");
    auto init = parseExpr();
    expect(TokenType::SEMICOLON, "expected ';' after let binding");

    return std::make_unique<LetStmtNode>(
        nameTok.lexeme, typeName, std::move(init), isMutable,
        letTok.line, letTok.column);
}

std::unique_ptr<ReturnStmtNode> Parser::parseReturnStmt() {
    const Token& retTok = advance(); // consume 'return'

    std::unique_ptr<Expr> value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = parseExpr();
    }

    expect(TokenType::SEMICOLON, "expected ';' after return");

    return std::make_unique<ReturnStmtNode>(
        std::move(value), retTok.line, retTok.column);
}

std::unique_ptr<IfStmtNode> Parser::parseIfStmt() {
    const Token& ifTok = advance(); // consume 'if'
    auto condition = parseExpr();
    auto thenBlock = parseBlock();

    std::unique_ptr<Stmt> elseBlock = nullptr;
    if (match(TokenType::KW_ELSE)) {
        if (check(TokenType::KW_IF)) {
            elseBlock = parseIfStmt();
        } else {
            elseBlock = parseBlock();
        }
    }

    return std::make_unique<IfStmtNode>(
        std::move(condition), std::move(thenBlock), std::move(elseBlock),
        ifTok.line, ifTok.column);
}

std::unique_ptr<WhileStmtNode> Parser::parseWhileStmt() {
    const Token& whileTok = advance(); // consume 'while'
    auto condition = parseExpr();
    auto body = parseBlock();

    return std::make_unique<WhileStmtNode>(
        std::move(condition), std::move(body),
        whileTok.line, whileTok.column);
}

std::unique_ptr<ForStmtNode> Parser::parseForStmt() {
    const Token& forTok = advance(); // consume 'for'
    const Token& varTok = expect(TokenType::IDENT, "expected variable name");
    expect(TokenType::KW_IN, "expected 'in' after for variable");
    auto iterable = parseExpr();
    auto body = parseBlock();

    return std::make_unique<ForStmtNode>(
        varTok.lexeme, std::move(iterable), std::move(body),
        forTok.line, forTok.column);
}

std::unique_ptr<ExprStmtNode> Parser::parseExprStmt() {
    int line = peek().line;
    int col = peek().column;
    auto expr = parseExpr();
    expect(TokenType::SEMICOLON, "expected ';' after expression");
    return std::make_unique<ExprStmtNode>(std::move(expr), line, col);
}

// ------------------------------------------------------------
// Expressions
// ------------------------------------------------------------

// Binary operator precedence: X(token_type, precedence_level)
// Higher number = tighter binding. Used by parseBinExpr().
#define BINARY_OPS(X)   \
    X(EQEQ,   2)       \
    X(BANGEQ,  2)       \
    X(LT,     3)       \
    X(GT,     3)       \
    X(LTEQ,   3)       \
    X(GTEQ,   3)       \
    X(PLUS,   4)       \
    X(MINUS,  4)       \
    X(STAR,   5)       \
    X(SLASH,  5)

static int getPrec(TokenType type) {
    switch (type) {
#define PREC_CASE(tok, prec) case TokenType::tok: return prec;
        BINARY_OPS(PREC_CASE)
#undef PREC_CASE
        default: return 0;
    }
}

std::unique_ptr<Expr> Parser::parseExpr() {
    return parseAssignment();
}

std::unique_ptr<Expr> Parser::parseAssignment() {
    auto left = parseBinExpr(2);

    if (match(TokenType::EQUALS)) {
        int line = previous().line;
        int col = previous().column;
        auto value = parseAssignment(); // right-associative
        return std::make_unique<AssignExprNode>(
            std::move(left), std::move(value), line, col);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseBinExpr(int minPrec) {
    auto left = parseUnary();

    while (getPrec(peek().type) >= minPrec) {
        const Token& opTok = advance();
        int nextMinPrec = getPrec(opTok.type) + 1; // left-associative
        auto right = parseBinExpr(nextMinPrec);
        left = std::make_unique<BinaryExprNode>(
            opTok.type, std::move(left), std::move(right),
            opTok.line, opTok.column);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (check(TokenType::MINUS)) {
        const Token& opTok = advance();
        auto operand = parsePostfix();
        return std::make_unique<UnaryExprNode>(
            opTok.type, std::move(operand), opTok.line, opTok.column);
    }
    return parsePostfix();
}

std::unique_ptr<Expr> Parser::parsePostfix() {
    auto expr = parsePrimary();

    while (true) {
        if (match(TokenType::LPAREN)) {
            int line = previous().line;
            int col = previous().column;
            auto args = parseArgList();
            expect(TokenType::RPAREN, "expected ')' after arguments");
            expr = std::make_unique<CallExprNode>(
                std::move(expr), std::move(args), line, col);
        } else if (match(TokenType::LBRACKET)) {
            int line = previous().line;
            int col = previous().column;
            auto index = parseExpr();
            expect(TokenType::RBRACKET, "expected ']' after index");
            expr = std::make_unique<IndexExprNode>(
                std::move(expr), std::move(index), line, col);
        } else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (check(TokenType::NUMBER)) {
        const Token& tok = advance();
        return std::make_unique<NumberLitNode>(tok.lexeme, tok.line, tok.column);
    }

    if (check(TokenType::STRING)) {
        const Token& tok = advance();
        return std::make_unique<StringLitNode>(tok.lexeme, tok.line, tok.column);
    }

    if (check(TokenType::IDENT)) {
        const Token& tok = advance();
        return std::make_unique<IdentNode>(tok.lexeme, tok.line, tok.column);
    }

    if (match(TokenType::LPAREN)) {
        auto expr = parseExpr();
        expect(TokenType::RPAREN, "expected ')' after expression");
        return expr;
    }

    throw ParseError("expected expression", peek());
}

std::vector<std::unique_ptr<Expr>> Parser::parseArgList() {
    std::vector<std::unique_ptr<Expr>> args;
    if (check(TokenType::RPAREN)) {
        return args;
    }

    do {
        args.push_back(parseExpr());
    } while (match(TokenType::COMMA));

    return args;
}
