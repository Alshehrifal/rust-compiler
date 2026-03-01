#pragma once

#include "token.h"
#include <memory>
#include <string>
#include <vector>

// X-macro for all AST node kinds
#define NODE_KINDS(X)    \
    /* Expressions */    \
    X(NumberLit)         \
    X(StringLit)         \
    X(Ident)             \
    X(UnaryExpr)         \
    X(BinaryExpr)        \
    X(AssignExpr)        \
    X(CallExpr)          \
    X(IndexExpr)         \
    /* Statements */     \
    X(ExprStmt)          \
    X(LetStmt)           \
    X(ReturnStmt)        \
    X(Block)             \
    X(IfStmt)            \
    X(WhileStmt)         \
    X(ForStmt)           \
    /* Declarations */   \
    X(FuncDecl)          \
    X(Program)

enum class NodeKind {
#define DEFINE_KIND(name) name,
    NODE_KINDS(DEFINE_KIND)
#undef DEFINE_KIND
};

inline const char* nodeKindName(NodeKind k) {
    switch (k) {
#define KIND_CASE(name) case NodeKind::name: return #name;
        NODE_KINDS(KIND_CASE)
#undef KIND_CASE
    }
    return "UNKNOWN";
}

// ------------------------------------------------------------
// Base classes
// ------------------------------------------------------------

struct ASTNode {
    int line;
    int column;

    ASTNode(int line, int column) : line(line), column(column) {}
    virtual ~ASTNode() = default;
    virtual NodeKind kind() const = 0;
};

struct Expr : ASTNode {
    using ASTNode::ASTNode;
};

struct Stmt : ASTNode {
    using ASTNode::ASTNode;
};

// Eliminates boilerplate kind() override in every node struct
#define AST_KIND(name) NodeKind kind() const override { return NodeKind::name; }

// ------------------------------------------------------------
// Expressions
// ------------------------------------------------------------

struct NumberLitNode : Expr {
    std::string value;

    NumberLitNode(const std::string& value, int line, int col)
        : Expr(line, col), value(value) {}
    AST_KIND(NumberLit)
};

struct StringLitNode : Expr {
    std::string value;

    StringLitNode(const std::string& value, int line, int col)
        : Expr(line, col), value(value) {}
    AST_KIND(StringLit)
};

struct IdentNode : Expr {
    std::string name;

    IdentNode(const std::string& name, int line, int col)
        : Expr(line, col), name(name) {}
    AST_KIND(Ident)
};

struct UnaryExprNode : Expr {
    TokenType op;
    std::unique_ptr<Expr> operand;

    UnaryExprNode(TokenType op, std::unique_ptr<Expr> operand, int line, int col)
        : Expr(line, col), op(op), operand(std::move(operand)) {}
    AST_KIND(UnaryExpr)
};

struct BinaryExprNode : Expr {
    TokenType op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    BinaryExprNode(TokenType op, std::unique_ptr<Expr> left,
                   std::unique_ptr<Expr> right, int line, int col)
        : Expr(line, col), op(op), left(std::move(left)),
          right(std::move(right)) {}
    AST_KIND(BinaryExpr)
};

struct AssignExprNode : Expr {
    std::unique_ptr<Expr> target;
    std::unique_ptr<Expr> value;

    AssignExprNode(std::unique_ptr<Expr> target, std::unique_ptr<Expr> value,
                   int line, int col)
        : Expr(line, col), target(std::move(target)),
          value(std::move(value)) {}
    AST_KIND(AssignExpr)
};

struct CallExprNode : Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;

    CallExprNode(std::unique_ptr<Expr> callee,
                 std::vector<std::unique_ptr<Expr>> args, int line, int col)
        : Expr(line, col), callee(std::move(callee)),
          args(std::move(args)) {}
    AST_KIND(CallExpr)
};

struct IndexExprNode : Expr {
    std::unique_ptr<Expr> object;
    std::unique_ptr<Expr> index;

    IndexExprNode(std::unique_ptr<Expr> object, std::unique_ptr<Expr> index,
                  int line, int col)
        : Expr(line, col), object(std::move(object)),
          index(std::move(index)) {}
    AST_KIND(IndexExpr)
};

// ------------------------------------------------------------
// Statements
// ------------------------------------------------------------

struct ExprStmtNode : Stmt {
    std::unique_ptr<Expr> expr;

    ExprStmtNode(std::unique_ptr<Expr> expr, int line, int col)
        : Stmt(line, col), expr(std::move(expr)) {}
    AST_KIND(ExprStmt)
};

struct BlockNode : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;

    BlockNode(std::vector<std::unique_ptr<Stmt>> statements, int line, int col)
        : Stmt(line, col), statements(std::move(statements)) {}
    AST_KIND(Block)
};

struct LetStmtNode : Stmt {
    std::string name;
    std::string typeName;  // empty if not specified
    std::unique_ptr<Expr> init;
    bool isMutable;

    LetStmtNode(const std::string& name, const std::string& typeName,
                std::unique_ptr<Expr> init, bool isMutable, int line, int col)
        : Stmt(line, col), name(name), typeName(typeName),
          init(std::move(init)), isMutable(isMutable) {}
    AST_KIND(LetStmt)
};

struct ReturnStmtNode : Stmt {
    std::unique_ptr<Expr> value;  // nullptr for bare return

    ReturnStmtNode(std::unique_ptr<Expr> value, int line, int col)
        : Stmt(line, col), value(std::move(value)) {}
    AST_KIND(ReturnStmt)
};

struct IfStmtNode : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockNode> thenBlock;
    std::unique_ptr<Stmt> elseBlock;  // BlockNode or IfStmtNode, or nullptr

    IfStmtNode(std::unique_ptr<Expr> condition,
               std::unique_ptr<BlockNode> thenBlock,
               std::unique_ptr<Stmt> elseBlock, int line, int col)
        : Stmt(line, col), condition(std::move(condition)),
          thenBlock(std::move(thenBlock)),
          elseBlock(std::move(elseBlock)) {}
    AST_KIND(IfStmt)
};

struct WhileStmtNode : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockNode> body;

    WhileStmtNode(std::unique_ptr<Expr> condition,
                  std::unique_ptr<BlockNode> body, int line, int col)
        : Stmt(line, col), condition(std::move(condition)),
          body(std::move(body)) {}
    AST_KIND(WhileStmt)
};

struct ForStmtNode : Stmt {
    std::string variable;
    std::unique_ptr<Expr> iterable;
    std::unique_ptr<BlockNode> body;

    ForStmtNode(const std::string& variable, std::unique_ptr<Expr> iterable,
                std::unique_ptr<BlockNode> body, int line, int col)
        : Stmt(line, col), variable(variable), iterable(std::move(iterable)),
          body(std::move(body)) {}
    AST_KIND(ForStmt)
};

// ------------------------------------------------------------
// Declarations
// ------------------------------------------------------------

struct Param {
    std::string name;
    std::string typeName;
};

struct FuncDeclNode : Stmt {
    std::string name;
    std::vector<Param> params;
    std::string returnType;  // empty if not specified
    std::unique_ptr<BlockNode> body;

    FuncDeclNode(const std::string& name, std::vector<Param> params,
                 const std::string& returnType,
                 std::unique_ptr<BlockNode> body, int line, int col)
        : Stmt(line, col), name(name), params(std::move(params)),
          returnType(returnType), body(std::move(body)) {}
    AST_KIND(FuncDecl)
};

struct ProgramNode : ASTNode {
    std::vector<std::unique_ptr<FuncDeclNode>> functions;

    ProgramNode(std::vector<std::unique_ptr<FuncDeclNode>> functions,
                int line, int col)
        : ASTNode(line, col), functions(std::move(functions)) {}
    AST_KIND(Program)
};
