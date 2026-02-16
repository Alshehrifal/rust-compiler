#include "lexer.h"
#include <gtest/gtest.h>
#include <sstream>
#include <vector>

// Helper: tokenize a string and return all tokens (including EOF)
static std::vector<Token> tokenize(const std::string& source) {
    std::istringstream stream(source);
    Lexer lexer(stream);
    std::vector<Token> tokens;
    while (lexer.hasNext()) {
        tokens.push_back(lexer.nextToken());
    }
    return tokens;
}

// Helper: tokenize and return all tokens before EOF
static std::vector<Token> tokenizeNoEof(const std::string& source) {
    auto tokens = tokenize(source);
    if (!tokens.empty() && tokens.back().type == TokenType::EOF_TOK) {
        tokens.pop_back();
    }
    return tokens;
}

// ---- Test macros ----

// Assert source tokenizes to exactly one token of the expected type
#define ASSERT_SINGLE_TOKEN(source, expected_type)          \
    do {                                                    \
        auto tokens = tokenizeNoEof(source);                \
        ASSERT_EQ(tokens.size(), 1u);                       \
        EXPECT_EQ(tokens[0].type, TokenType::expected_type);\
    } while (0)

// Assert source tokenizes to one token with expected type AND lexeme
#define ASSERT_SINGLE_TOKEN_LEXEME(source, expected_type, expected_lexeme)   \
    do {                                                                     \
        auto tokens = tokenizeNoEof(source);                                 \
        ASSERT_EQ(tokens.size(), 1u);                                        \
        EXPECT_EQ(tokens[0].type, TokenType::expected_type);                 \
        EXPECT_EQ(tokens[0].lexeme, expected_lexeme);                        \
    } while (0)

// Generate a full TEST() for a keyword (source text == expected lexeme)
#define TEST_KEYWORD(test_name, keyword_str, token_type)    \
    TEST(LexerKeywords, test_name) {                        \
        ASSERT_SINGLE_TOKEN_LEXEME(keyword_str, token_type, keyword_str); \
    }

// Generate a full TEST() for a single-char token (type check only)
#define TEST_TOKEN(group, test_name, source, token_type)    \
    TEST(group, test_name) {                                \
        ASSERT_SINGLE_TOKEN(source, token_type);            \
    }

// Generate a full TEST() for a token with lexeme check
#define TEST_TOKEN_LEXEME(group, test_name, source, token_type, lexeme) \
    TEST(group, test_name) {                                            \
        ASSERT_SINGLE_TOKEN_LEXEME(source, token_type, lexeme);         \
    }

// ============================================================
// Keywords -- 8 tests generated from 8 macro calls
// ============================================================

TEST_KEYWORD(Fn,     "fn",     KW_FN)
TEST_KEYWORD(Let,    "let",    KW_LET)
TEST_KEYWORD(Mut,    "mut",    KW_MUT)
TEST_KEYWORD(If,     "if",     KW_IF)
TEST_KEYWORD(Else,   "else",   KW_ELSE)
TEST_KEYWORD(While,  "while",  KW_WHILE)
TEST_KEYWORD(Return, "return", KW_RETURN)
TEST_KEYWORD(Struct, "struct", KW_STRUCT)

// ============================================================
// Identifiers
// ============================================================

TEST_TOKEN_LEXEME(LexerIdentifiers, Simple,           "foo",        IDENT, "foo")
TEST_TOKEN_LEXEME(LexerIdentifiers, UnderscorePrefixed,"_bar",      IDENT, "_bar")
TEST_TOKEN_LEXEME(LexerIdentifiers, MixedCase,        "myVar_123",  IDENT, "myVar_123")
TEST_TOKEN_LEXEME(LexerIdentifiers, KeywordPrefix,    "fn_name",    IDENT, "fn_name")

// ============================================================
// Numbers
// ============================================================

TEST_TOKEN_LEXEME(LexerNumbers, SimpleInteger,        "42",         NUMBER, "42")
TEST_TOKEN_LEXEME(LexerNumbers, Zero,                 "0",          NUMBER, "0")
TEST_TOKEN_LEXEME(LexerNumbers, UnderscoreSeparated,  "3_141_592",  NUMBER, "3_141_592")

// ============================================================
// Strings
// ============================================================

TEST_TOKEN_LEXEME(LexerStrings, Simple,    "\"hello\"",                STRING, "hello")
TEST_TOKEN_LEXEME(LexerStrings, EmptyString,"\"\"",                    STRING, "")
TEST_TOKEN_LEXEME(LexerStrings, MultiWord, "\"hello world\"",          STRING, "hello world")

TEST(LexerStrings, EscapeSequences) {
    ASSERT_SINGLE_TOKEN_LEXEME("\"a\\nb\\tc\\\\d\\\"e\"", STRING, "a\nb\tc\\d\"e");
}

// ============================================================
// Arithmetic Operators
// ============================================================

TEST_TOKEN(LexerOperators, Plus,  "+", PLUS)
TEST_TOKEN(LexerOperators, Minus, "-", MINUS)
TEST_TOKEN(LexerOperators, Star,  "*", STAR)
TEST_TOKEN(LexerOperators, SlashOp, "/", SLASH)

// ============================================================
// Assignment
// ============================================================

TEST_TOKEN(LexerOperators, Equals, "=", EQUALS)

// ============================================================
// Comparison Operators
// ============================================================

TEST_TOKEN_LEXEME(LexerComparison, EqEq,   "==", EQEQ,   "==")
TEST_TOKEN_LEXEME(LexerComparison, BangEq, "!=", BANGEQ,  "!=")
TEST_TOKEN(LexerComparison, Lt, "<", LT)
TEST_TOKEN(LexerComparison, Gt, ">", GT)
TEST_TOKEN_LEXEME(LexerComparison, LtEq, "<=", LTEQ, "<=")
TEST_TOKEN_LEXEME(LexerComparison, GtEq, ">=", GTEQ, ">=")

// ============================================================
// Arrow
// ============================================================

TEST_TOKEN_LEXEME(LexerOperators, Arrow, "->", ARROW, "->")

// ============================================================
// Punctuation -- 9 tests generated from 9 macro calls
// ============================================================

TEST_TOKEN(LexerPunctuation, LParen,    "(", LPAREN)
TEST_TOKEN(LexerPunctuation, RParen,    ")", RPAREN)
TEST_TOKEN(LexerPunctuation, LBrace,    "{", LBRACE)
TEST_TOKEN(LexerPunctuation, RBrace,    "}", RBRACE)
TEST_TOKEN(LexerPunctuation, LBracket,  "[", LBRACKET)
TEST_TOKEN(LexerPunctuation, RBracket,  "]", RBRACKET)
TEST_TOKEN(LexerPunctuation, Semicolon, ";", SEMICOLON)
TEST_TOKEN(LexerPunctuation, Colon,     ":", COLON)
TEST_TOKEN(LexerPunctuation, Comma,     ",", COMMA)

// ============================================================
// Whitespace Handling
// ============================================================

TEST(LexerWhitespace, SkipsSpacesTabsNewlines) {
    auto tokens = tokenizeNoEof("  fn  \t  let  \n  mut  ");
    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].type, TokenType::KW_FN);
    EXPECT_EQ(tokens[1].type, TokenType::KW_LET);
    EXPECT_EQ(tokens[2].type, TokenType::KW_MUT);
}

TEST(LexerWhitespace, LineTracking) {
    auto tokens = tokenizeNoEof("fn\nlet\nmut");
    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].line, 1);
    EXPECT_EQ(tokens[1].line, 2);
    EXPECT_EQ(tokens[2].line, 3);
}

// ============================================================
// Combined Tokens
// ============================================================

TEST(LexerCombined, LetBinding) {
    auto tokens = tokenizeNoEof("let x = 42;");
    ASSERT_EQ(tokens.size(), 5u);
    EXPECT_EQ(tokens[0].type, TokenType::KW_LET);
    EXPECT_EQ(tokens[1].type, TokenType::IDENT);
    EXPECT_EQ(tokens[1].lexeme, "x");
    EXPECT_EQ(tokens[2].type, TokenType::EQUALS);
    EXPECT_EQ(tokens[3].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[3].lexeme, "42");
    EXPECT_EQ(tokens[4].type, TokenType::SEMICOLON);
}

TEST(LexerCombined, FnMain) {
    auto tokens = tokenizeNoEof("fn main() {}");
    ASSERT_EQ(tokens.size(), 6u);
    EXPECT_EQ(tokens[0].type, TokenType::KW_FN);
    EXPECT_EQ(tokens[1].type, TokenType::IDENT);
    EXPECT_EQ(tokens[1].lexeme, "main");
    EXPECT_EQ(tokens[2].type, TokenType::LPAREN);
    EXPECT_EQ(tokens[3].type, TokenType::RPAREN);
    EXPECT_EQ(tokens[4].type, TokenType::LBRACE);
    EXPECT_EQ(tokens[5].type, TokenType::RBRACE);
}

TEST(LexerCombined, FullFunction) {
    auto tokens = tokenizeNoEof(
        "fn add(a: i32, b: i32) -> i32 {\n"
        "    return a + b;\n"
        "}"
    );
    ASSERT_EQ(tokens.size(), 20u);
    EXPECT_EQ(tokens[0].type, TokenType::KW_FN);
    EXPECT_EQ(tokens[1].type, TokenType::IDENT);
    EXPECT_EQ(tokens[1].lexeme, "add");
    EXPECT_EQ(tokens[2].type, TokenType::LPAREN);
    EXPECT_EQ(tokens[3].type, TokenType::IDENT);  // a
    EXPECT_EQ(tokens[4].type, TokenType::COLON);
    EXPECT_EQ(tokens[5].type, TokenType::IDENT);  // i32
    EXPECT_EQ(tokens[6].type, TokenType::COMMA);
    EXPECT_EQ(tokens[7].type, TokenType::IDENT);  // b
    EXPECT_EQ(tokens[8].type, TokenType::COLON);
    EXPECT_EQ(tokens[9].type, TokenType::IDENT);  // i32
    EXPECT_EQ(tokens[10].type, TokenType::RPAREN);
    EXPECT_EQ(tokens[11].type, TokenType::ARROW);
    EXPECT_EQ(tokens[12].type, TokenType::IDENT); // i32
    EXPECT_EQ(tokens[13].type, TokenType::LBRACE);
    EXPECT_EQ(tokens[14].type, TokenType::KW_RETURN);
    EXPECT_EQ(tokens[15].type, TokenType::IDENT); // a
    EXPECT_EQ(tokens[16].type, TokenType::PLUS);
    EXPECT_EQ(tokens[17].type, TokenType::IDENT); // b
    EXPECT_EQ(tokens[18].type, TokenType::SEMICOLON);
    EXPECT_EQ(tokens[19].type, TokenType::RBRACE);
}

// ============================================================
// Edge Cases
// ============================================================

TEST(LexerEdgeCases, EmptyInputReturnsEof) {
    auto tokens = tokenize("");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::EOF_TOK);
}

TEST(LexerEdgeCases, EqEqIsNotTwoEquals) {
    ASSERT_SINGLE_TOKEN("==", EQEQ);
}

TEST(LexerEdgeCases, ArrowIsNotMinusThenGt) {
    ASSERT_SINGLE_TOKEN("->", ARROW);
}

TEST_TOKEN_LEXEME(LexerEdgeCases, FnNameIsIdentNotKeyword, "fn_name", IDENT, "fn_name")

TEST(LexerEdgeCases, ConsecutiveOperatorsNoWhitespace) {
    auto tokens = tokenizeNoEof("+-*/");
    ASSERT_EQ(tokens.size(), 4u);
    EXPECT_EQ(tokens[0].type, TokenType::PLUS);
    EXPECT_EQ(tokens[1].type, TokenType::MINUS);
    EXPECT_EQ(tokens[2].type, TokenType::STAR);
    EXPECT_EQ(tokens[3].type, TokenType::SLASH);
}

TEST_TOKEN(LexerEdgeCases, UnknownCharProducesError, "@", ERROR)

TEST(LexerEdgeCases, ColumnTracking) {
    auto tokens = tokenizeNoEof("let x");
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].column, 1);
    EXPECT_EQ(tokens[1].column, 5);
}

TEST_TOKEN(LexerEdgeCases, LoneBangIsError, "!", ERROR)

TEST_TOKEN_LEXEME(LexerEdgeCases, LetterIsIdentNotKeyword, "letter", IDENT, "letter")
