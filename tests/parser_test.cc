#include "parser.h"
#include "lexer.h"
#include <gtest/gtest.h>
#include <sstream>

// ============================================================
// Helpers
// ============================================================

static std::vector<Token> tokenize(const std::string& source) {
    std::istringstream stream(source);
    Lexer lexer(stream);
    std::vector<Token> tokens;
    while (lexer.hasNext()) {
        tokens.push_back(lexer.nextToken());
    }
    return tokens;
}

static std::unique_ptr<Expr> parseExpr(const std::string& source) {
    std::string wrapped = "fn _() { " + source + "; }";
    auto tokens = tokenize(wrapped);
    Parser parser(tokens);
    auto program = parser.parse();
    auto& body = program->functions[0]->body->statements;
    auto* exprStmt = dynamic_cast<ExprStmtNode*>(body[0].get());
    return std::unique_ptr<Expr>(exprStmt->expr.release());
}

static std::unique_ptr<ProgramNode> parseProgram(const std::string& source) {
    auto tokens = tokenize(source);
    Parser parser(tokens);
    return parser.parse();
}

// ---- Test macros ----

// Assert source fails to parse
#define ASSERT_PARSE_ERROR(source) \
    EXPECT_THROW({ parseProgram(source); }, ParseError)

// Assert expression parses to a BinaryExpr with expected operator
#define ASSERT_BINARY_OP(source, expected_op)                           \
    do {                                                                \
        auto expr = parseExpr(source);                                  \
        ASSERT_EQ(expr->kind(), NodeKind::BinaryExpr);                  \
        auto* bin = dynamic_cast<BinaryExprNode*>(expr.get());          \
        EXPECT_EQ(bin->op, TokenType::expected_op);                     \
    } while (0)

// ============================================================
// Smoke test -- parser skeleton compiles and runs
// ============================================================

TEST(ParserSmoke, EmptyProgramParses) {
    auto tokens = tokenize("");
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT_NE(program, nullptr);
    EXPECT_EQ(program->kind(), NodeKind::Program);
    EXPECT_EQ(program->functions.size(), 0u);
}

// ============================================================
// Primary expressions
// ============================================================

TEST(ParserPrimary, NumberLiteral) {
    auto expr = parseExpr("42");
    ASSERT_EQ(expr->kind(), NodeKind::NumberLit);
    auto* num = dynamic_cast<NumberLitNode*>(expr.get());
    EXPECT_EQ(num->value, "42");
}

TEST(ParserPrimary, StringLiteral) {
    auto expr = parseExpr("\"hello\"");
    ASSERT_EQ(expr->kind(), NodeKind::StringLit);
    auto* str = dynamic_cast<StringLitNode*>(expr.get());
    EXPECT_EQ(str->value, "hello");
}

TEST(ParserPrimary, Identifier) {
    auto expr = parseExpr("foo");
    ASSERT_EQ(expr->kind(), NodeKind::Ident);
    auto* ident = dynamic_cast<IdentNode*>(expr.get());
    EXPECT_EQ(ident->name, "foo");
}

TEST(ParserPrimary, GroupedExpression) {
    auto expr = parseExpr("(42)");
    ASSERT_EQ(expr->kind(), NodeKind::NumberLit);
    auto* num = dynamic_cast<NumberLitNode*>(expr.get());
    EXPECT_EQ(num->value, "42");
}

TEST(ParserPrimary, MissingCloseParen) { ASSERT_PARSE_ERROR("fn _() { (42; }"); }
TEST(ParserPrimary, EmptyInputExpression) { ASSERT_PARSE_ERROR("fn _() { ; }"); }

// ============================================================
// Expression parsing with precedence
// ============================================================

TEST(ParserExpr, BinaryAddition) {
    auto expr = parseExpr("1 + 2");
    ASSERT_EQ(expr->kind(), NodeKind::BinaryExpr);
    auto* bin = dynamic_cast<BinaryExprNode*>(expr.get());
    EXPECT_EQ(bin->op, TokenType::PLUS);
    EXPECT_EQ(bin->left->kind(), NodeKind::NumberLit);
    EXPECT_EQ(bin->right->kind(), NodeKind::NumberLit);
}

TEST(ParserExpr, PrecedenceMultOverAdd) {
    // 1 + 2 * 3 should parse as 1 + (2 * 3)
    auto expr = parseExpr("1 + 2 * 3");
    ASSERT_EQ(expr->kind(), NodeKind::BinaryExpr);
    auto* add = dynamic_cast<BinaryExprNode*>(expr.get());
    EXPECT_EQ(add->op, TokenType::PLUS);
    EXPECT_EQ(add->left->kind(), NodeKind::NumberLit);
    ASSERT_EQ(add->right->kind(), NodeKind::BinaryExpr);
    auto* mult = dynamic_cast<BinaryExprNode*>(add->right.get());
    EXPECT_EQ(mult->op, TokenType::STAR);
}

TEST(ParserExpr, LeftAssociativity) {
    // 1 - 2 - 3 should parse as (1 - 2) - 3
    auto expr = parseExpr("1 - 2 - 3");
    ASSERT_EQ(expr->kind(), NodeKind::BinaryExpr);
    auto* outer = dynamic_cast<BinaryExprNode*>(expr.get());
    EXPECT_EQ(outer->op, TokenType::MINUS);
    ASSERT_EQ(outer->left->kind(), NodeKind::BinaryExpr);
    auto* inner = dynamic_cast<BinaryExprNode*>(outer->left.get());
    EXPECT_EQ(inner->op, TokenType::MINUS);
    EXPECT_EQ(outer->right->kind(), NodeKind::NumberLit);
}

// Binary operator tests -- generated from X-macro
// Covers all comparison and equality operators.
#define BINARY_OP_TESTS(X)                             \
    X(EqualityEqEq,     "a == b",  EQEQ)              \
    X(EqualityBangEq,   "a != b",  BANGEQ)            \
    X(ComparisonLt,     "a < b",   LT)                \
    X(ComparisonGt,     "a > b",   GT)                \
    X(ComparisonLtEq,   "a <= b",  LTEQ)              \
    X(ComparisonGtEq,   "a >= b",  GTEQ)

#define GEN_BINARY_OP_TEST(name, source, op) \
    TEST(ParserExpr, name) { ASSERT_BINARY_OP(source, op); }
BINARY_OP_TESTS(GEN_BINARY_OP_TEST)
#undef GEN_BINARY_OP_TEST

TEST(ParserExpr, UnaryNegation) {
    auto expr = parseExpr("-42");
    ASSERT_EQ(expr->kind(), NodeKind::UnaryExpr);
    auto* unary = dynamic_cast<UnaryExprNode*>(expr.get());
    EXPECT_EQ(unary->op, TokenType::MINUS);
    EXPECT_EQ(unary->operand->kind(), NodeKind::NumberLit);
}

TEST(ParserExpr, FunctionCall) {
    auto expr = parseExpr("foo(1, 2)");
    ASSERT_EQ(expr->kind(), NodeKind::CallExpr);
    auto* call = dynamic_cast<CallExprNode*>(expr.get());
    EXPECT_EQ(call->callee->kind(), NodeKind::Ident);
    EXPECT_EQ(call->args.size(), 2u);
}

TEST(ParserExpr, EmptyFunctionCall) {
    auto expr = parseExpr("foo()");
    ASSERT_EQ(expr->kind(), NodeKind::CallExpr);
    auto* call = dynamic_cast<CallExprNode*>(expr.get());
    EXPECT_EQ(call->args.size(), 0u);
}

TEST(ParserExpr, ArrayIndex) {
    auto expr = parseExpr("arr[0]");
    ASSERT_EQ(expr->kind(), NodeKind::IndexExpr);
    auto* index = dynamic_cast<IndexExprNode*>(expr.get());
    EXPECT_EQ(index->object->kind(), NodeKind::Ident);
    EXPECT_EQ(index->index->kind(), NodeKind::NumberLit);
}

TEST(ParserExpr, ChainedArrayIndex) {
    // m[i][j] should parse as (m[i])[j]
    auto expr = parseExpr("m[i][j]");
    ASSERT_EQ(expr->kind(), NodeKind::IndexExpr);
    auto* outer = dynamic_cast<IndexExprNode*>(expr.get());
    ASSERT_EQ(outer->object->kind(), NodeKind::IndexExpr);
    auto* inner = dynamic_cast<IndexExprNode*>(outer->object.get());
    EXPECT_EQ(inner->object->kind(), NodeKind::Ident);
    EXPECT_EQ(inner->index->kind(), NodeKind::Ident);
    EXPECT_EQ(outer->index->kind(), NodeKind::Ident);
}

TEST(ParserExpr, ArrayIndexWithBinaryExpr) {
    // a[i + 1] -- index is a binary expression
    auto expr = parseExpr("a[i + 1]");
    ASSERT_EQ(expr->kind(), NodeKind::IndexExpr);
    auto* idx = dynamic_cast<IndexExprNode*>(expr.get());
    EXPECT_EQ(idx->object->kind(), NodeKind::Ident);
    ASSERT_EQ(idx->index->kind(), NodeKind::BinaryExpr);
    auto* bin = dynamic_cast<BinaryExprNode*>(idx->index.get());
    EXPECT_EQ(bin->op, TokenType::PLUS);
}

TEST(ParserExpr, Assignment) {
    auto expr = parseExpr("x = 42");
    ASSERT_EQ(expr->kind(), NodeKind::AssignExpr);
    auto* assign = dynamic_cast<AssignExprNode*>(expr.get());
    EXPECT_EQ(assign->target->kind(), NodeKind::Ident);
    EXPECT_EQ(assign->value->kind(), NodeKind::NumberLit);
}

TEST(ParserExpr, AssignmentRightAssociative) {
    // a = b = c should parse as a = (b = c)
    auto expr = parseExpr("a = b = c");
    ASSERT_EQ(expr->kind(), NodeKind::AssignExpr);
    auto* outer = dynamic_cast<AssignExprNode*>(expr.get());
    EXPECT_EQ(outer->target->kind(), NodeKind::Ident);
    ASSERT_EQ(outer->value->kind(), NodeKind::AssignExpr);
    auto* inner = dynamic_cast<AssignExprNode*>(outer->value.get());
    EXPECT_EQ(inner->target->kind(), NodeKind::Ident);
    EXPECT_EQ(inner->value->kind(), NodeKind::Ident);
}

TEST(ParserExpr, ChainedCalls) {
    // foo(1)(2) -- callee of second call is first call
    auto expr = parseExpr("foo(1)(2)");
    ASSERT_EQ(expr->kind(), NodeKind::CallExpr);
    auto* outer = dynamic_cast<CallExprNode*>(expr.get());
    ASSERT_EQ(outer->callee->kind(), NodeKind::CallExpr);
    EXPECT_EQ(outer->args.size(), 1u);
}

TEST(ParserExpr, ComplexPrecedence) {
    // -a + b * c == d parses as ((-a) + (b * c)) == d
    auto expr = parseExpr("-a + b * c == d"); // NOLINT
    ASSERT_EQ(expr->kind(), NodeKind::BinaryExpr);
    auto* eq = dynamic_cast<BinaryExprNode*>(expr.get());
    EXPECT_EQ(eq->op, TokenType::EQEQ);
    ASSERT_EQ(eq->left->kind(), NodeKind::BinaryExpr);
    auto* add = dynamic_cast<BinaryExprNode*>(eq->left.get());
    EXPECT_EQ(add->op, TokenType::PLUS);
    EXPECT_EQ(add->left->kind(), NodeKind::UnaryExpr);
    ASSERT_EQ(add->right->kind(), NodeKind::BinaryExpr);
    auto* mult = dynamic_cast<BinaryExprNode*>(add->right.get());
    EXPECT_EQ(mult->op, TokenType::STAR);
}

// ============================================================
// Statement parsing
// ============================================================

TEST(ParserStmt, LetBinding) {
    auto program = parseProgram("fn _() { let x = 42; }");
    auto& stmts = program->functions[0]->body->statements;
    ASSERT_EQ(stmts.size(), 1u);
    ASSERT_EQ(stmts[0]->kind(), NodeKind::LetStmt);
    auto* let = dynamic_cast<LetStmtNode*>(stmts[0].get());
    EXPECT_EQ(let->name, "x");
    EXPECT_FALSE(let->isMutable);
    EXPECT_TRUE(let->typeName.empty());
    EXPECT_EQ(let->init->kind(), NodeKind::NumberLit);
}

TEST(ParserStmt, LetMutWithType) {
    auto program = parseProgram("fn _() { let mut x: i32 = 42; }");
    auto& stmts = program->functions[0]->body->statements;
    ASSERT_EQ(stmts.size(), 1u);
    auto* let = dynamic_cast<LetStmtNode*>(stmts[0].get());
    EXPECT_EQ(let->name, "x");
    EXPECT_TRUE(let->isMutable);
    EXPECT_EQ(let->typeName, "i32");
}

TEST(ParserStmt, ReturnWithValue) {
    auto program = parseProgram("fn _() { return 42; }");
    auto& stmts = program->functions[0]->body->statements;
    ASSERT_EQ(stmts.size(), 1u);
    ASSERT_EQ(stmts[0]->kind(), NodeKind::ReturnStmt);
    auto* ret = dynamic_cast<ReturnStmtNode*>(stmts[0].get());
    ASSERT_NE(ret->value, nullptr);
    EXPECT_EQ(ret->value->kind(), NodeKind::NumberLit);
}

TEST(ParserStmt, BareReturn) {
    auto program = parseProgram("fn _() { return; }");
    auto& stmts = program->functions[0]->body->statements;
    ASSERT_EQ(stmts.size(), 1u);
    auto* ret = dynamic_cast<ReturnStmtNode*>(stmts[0].get());
    EXPECT_EQ(ret->value, nullptr);
}

TEST(ParserStmt, IfStatement) {
    auto program = parseProgram("fn _() { if x { y; } }");
    auto& stmts = program->functions[0]->body->statements;
    ASSERT_EQ(stmts.size(), 1u);
    ASSERT_EQ(stmts[0]->kind(), NodeKind::IfStmt);
    auto* ifStmt = dynamic_cast<IfStmtNode*>(stmts[0].get());
    EXPECT_NE(ifStmt->condition, nullptr);
    EXPECT_NE(ifStmt->thenBlock, nullptr);
    EXPECT_EQ(ifStmt->elseBlock, nullptr);
}

TEST(ParserStmt, IfElse) {
    auto program = parseProgram("fn _() { if x { y; } else { z; } }");
    auto& stmts = program->functions[0]->body->statements;
    ASSERT_EQ(stmts.size(), 1u);
    auto* ifStmt = dynamic_cast<IfStmtNode*>(stmts[0].get());
    ASSERT_NE(ifStmt->elseBlock, nullptr);
    EXPECT_EQ(ifStmt->elseBlock->kind(), NodeKind::Block);
}

TEST(ParserStmt, IfElseIf) {
    auto program = parseProgram("fn _() { if a { b; } else if c { d; } }");
    auto& stmts = program->functions[0]->body->statements;
    ASSERT_EQ(stmts.size(), 1u);
    auto* ifStmt = dynamic_cast<IfStmtNode*>(stmts[0].get());
    ASSERT_NE(ifStmt->elseBlock, nullptr);
    EXPECT_EQ(ifStmt->elseBlock->kind(), NodeKind::IfStmt);
}

TEST(ParserStmt, WhileLoop) {
    auto program = parseProgram("fn _() { while x { y; } }");
    auto& stmts = program->functions[0]->body->statements;
    ASSERT_EQ(stmts.size(), 1u);
    ASSERT_EQ(stmts[0]->kind(), NodeKind::WhileStmt);
    auto* whileStmt = dynamic_cast<WhileStmtNode*>(stmts[0].get());
    EXPECT_NE(whileStmt->condition, nullptr);
    EXPECT_NE(whileStmt->body, nullptr);
}

TEST(ParserStmt, ForLoop) {
    auto program = parseProgram("fn _() { for i in items { x; } }");
    auto& stmts = program->functions[0]->body->statements;
    ASSERT_EQ(stmts.size(), 1u);
    ASSERT_EQ(stmts[0]->kind(), NodeKind::ForStmt);
    auto* forStmt = dynamic_cast<ForStmtNode*>(stmts[0].get());
    EXPECT_EQ(forStmt->variable, "i");
    EXPECT_NE(forStmt->iterable, nullptr);
    EXPECT_NE(forStmt->body, nullptr);
}

TEST(ParserStmt, ExpressionStatement) {
    auto program = parseProgram("fn _() { foo; }");
    auto& stmts = program->functions[0]->body->statements;
    ASSERT_EQ(stmts.size(), 1u);
    ASSERT_EQ(stmts[0]->kind(), NodeKind::ExprStmt);
}

TEST(ParserStmt, MultipleStatements) {
    auto program = parseProgram("fn _() { let x = 1; let y = 2; return x; }");
    auto& stmts = program->functions[0]->body->statements;
    EXPECT_EQ(stmts.size(), 3u);
    EXPECT_EQ(stmts[0]->kind(), NodeKind::LetStmt);
    EXPECT_EQ(stmts[1]->kind(), NodeKind::LetStmt);
    EXPECT_EQ(stmts[2]->kind(), NodeKind::ReturnStmt);
}

// ============================================================
// Error cases -- generated from X-macro
// ============================================================

#define PARSE_ERROR_TESTS(X)                                                    \
    X(ParserErrors, MissingSemicolon,    "fn _() { let x = 42 }")              \
    X(ParserErrors, MissingEquals,       "fn _() { let x 42; }")              \
    X(ParserErrors, MissingBrace,        "fn _() { if x y; }")                \
    X(ParserErrors, MissingInKeyword,    "fn _() { for i items { x; } }")

#define GEN_PARSE_ERROR_TEST(group, name, source) \
    TEST(group, name) { ASSERT_PARSE_ERROR(source); }
PARSE_ERROR_TESTS(GEN_PARSE_ERROR_TEST)
#undef GEN_PARSE_ERROR_TEST

// ============================================================
// Function and program parsing
// ============================================================

TEST(ParserDecl, EmptyFunction) {
    auto program = parseProgram("fn main() {}");
    ASSERT_EQ(program->functions.size(), 1u);
    auto& fn = program->functions[0];
    EXPECT_EQ(fn->name, "main");
    EXPECT_TRUE(fn->params.empty());
    EXPECT_TRUE(fn->returnType.empty());
    EXPECT_TRUE(fn->body->statements.empty());
}

TEST(ParserDecl, FunctionWithParams) {
    auto program = parseProgram("fn add(a: i32, b: i32) -> i32 { return a; }");
    ASSERT_EQ(program->functions.size(), 1u);
    auto& fn = program->functions[0];
    EXPECT_EQ(fn->name, "add");
    ASSERT_EQ(fn->params.size(), 2u);
    EXPECT_EQ(fn->params[0].name, "a");
    EXPECT_EQ(fn->params[0].typeName, "i32");
    EXPECT_EQ(fn->params[1].name, "b");
    EXPECT_EQ(fn->params[1].typeName, "i32");
    EXPECT_EQ(fn->returnType, "i32");
}

TEST(ParserDecl, MultipleFunctions) {
    auto program = parseProgram("fn foo() {} fn bar() {}");
    EXPECT_EQ(program->functions.size(), 2u);
    EXPECT_EQ(program->functions[0]->name, "foo");
    EXPECT_EQ(program->functions[1]->name, "bar");
}

TEST(ParserDecl, EmptyProgram) {
    auto program = parseProgram("");
    EXPECT_EQ(program->functions.size(), 0u);
}

#define DECL_ERROR_TESTS(X)                                                    \
    X(ParserDecl, MissingFnKeyword,      "main() {}")                          \
    X(ParserDecl, MissingFunctionName,   "fn () {}")                           \
    X(ParserDecl, MissingParamColon,     "fn foo(a i32) {}")                   \
    X(ParserDecl, MissingFunctionBody,   "fn foo()")

#define GEN_DECL_ERROR_TEST(group, name, source) \
    TEST(group, name) { ASSERT_PARSE_ERROR(source); }
DECL_ERROR_TESTS(GEN_DECL_ERROR_TEST)
#undef GEN_DECL_ERROR_TEST

// ============================================================
// Integration tests
// ============================================================

TEST(ParserIntegration, FullProgram) {
    auto program = parseProgram(
        "fn add(a: i32, b: i32) -> i32 {\n"
        "    return a + b;\n"
        "}\n"
        "fn main() {\n"
        "    let x = add(1, 2);\n"
        "    if x > 0 {\n"
        "        return;\n"
        "    }\n"
        "}\n"
    );
    ASSERT_EQ(program->functions.size(), 2u);
    EXPECT_EQ(program->functions[0]->name, "add");
    EXPECT_EQ(program->functions[1]->name, "main");

    // Check add function
    auto& addBody = program->functions[0]->body->statements;
    ASSERT_EQ(addBody.size(), 1u);
    EXPECT_EQ(addBody[0]->kind(), NodeKind::ReturnStmt);

    // Check main function
    auto& mainBody = program->functions[1]->body->statements;
    ASSERT_EQ(mainBody.size(), 2u);
    EXPECT_EQ(mainBody[0]->kind(), NodeKind::LetStmt);
    EXPECT_EQ(mainBody[1]->kind(), NodeKind::IfStmt);
}
