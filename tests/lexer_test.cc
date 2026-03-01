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

// Expected token for sequence assertions (lexeme is optional)
struct Expected {
    TokenType type;
    std::string lexeme;  // empty = don't check
};

// Assert source tokenizes to an exact sequence of tokens
static void assertTokenSequence(const std::string& source,
                                const std::vector<Expected>& expected) {
    auto tokens = tokenizeNoEof(source);
    ASSERT_EQ(tokens.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(tokens[i].type, expected[i].type)
            << "token[" << i << "] type mismatch";
        if (!expected[i].lexeme.empty()) {
            EXPECT_EQ(tokens[i].lexeme, expected[i].lexeme)
                << "token[" << i << "] lexeme mismatch";
        }
    }
}

// ============================================================
// Keywords -- generated from KEYWORD_TOKENS X-macro
// ============================================================

#define GEN_KEYWORD_TEST(name, str)                      \
    TEST(LexerKeywords, name) {                          \
        ASSERT_SINGLE_TOKEN_LEXEME(str, name, str);      \
    }
KEYWORD_TOKENS(GEN_KEYWORD_TEST)
#undef GEN_KEYWORD_TEST

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
// Single-char tokens -- generated from SINGLE_CHAR_TOKENS X-macro
// ============================================================

#define GEN_SINGLE_CHAR_TEST(name, ch)                         \
    TEST(LexerSingleChar, name) {                              \
        const std::string src(1, ch);                          \
        auto tokens = tokenizeNoEof(src);                      \
        ASSERT_EQ(tokens.size(), 1u);                          \
        EXPECT_EQ(tokens[0].type, TokenType::name);            \
    }
SINGLE_CHAR_TOKENS(GEN_SINGLE_CHAR_TEST)
#undef GEN_SINGLE_CHAR_TEST

// ============================================================
// Multi-char operators (require lookahead, tested manually)
// ============================================================

TEST_TOKEN(LexerOperators, Minus, "-", MINUS)
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
// Whitespace Handling
// ============================================================

TEST(LexerWhitespace, SkipsSpacesTabsNewlines) {
    assertTokenSequence("  fn  \t  let  \n  mut  ", {
        {TokenType::KW_FN,  {}},
        {TokenType::KW_LET, {}},
        {TokenType::KW_MUT, {}},
    });
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
    assertTokenSequence("let x = 42;", {
        {TokenType::KW_LET,   {}},
        {TokenType::IDENT,    "x"},
        {TokenType::EQUALS,   {}},
        {TokenType::NUMBER,   "42"},
        {TokenType::SEMICOLON,{}},
    });
}

TEST(LexerCombined, FnMain) {
    assertTokenSequence("fn main() {}", {
        {TokenType::KW_FN,  {}},
        {TokenType::IDENT,  "main"},
        {TokenType::LPAREN, {}},
        {TokenType::RPAREN, {}},
        {TokenType::LBRACE, {}},
        {TokenType::RBRACE, {}},
    });
}

TEST(LexerCombined, FullFunction) {
    assertTokenSequence(
        "fn add(a: i32, b: i32) -> i32 {\n"
        "    return a + b;\n"
        "}",
        {
            {TokenType::KW_FN,    {}},
            {TokenType::IDENT,    "add"},
            {TokenType::LPAREN,   {}},
            {TokenType::IDENT,    "a"},
            {TokenType::COLON,    {}},
            {TokenType::IDENT,    "i32"},
            {TokenType::COMMA,    {}},
            {TokenType::IDENT,    "b"},
            {TokenType::COLON,    {}},
            {TokenType::IDENT,    "i32"},
            {TokenType::RPAREN,   {}},
            {TokenType::ARROW,    {}},
            {TokenType::IDENT,    "i32"},
            {TokenType::LBRACE,   {}},
            {TokenType::KW_RETURN,{}},
            {TokenType::IDENT,    "a"},
            {TokenType::PLUS,     {}},
            {TokenType::IDENT,    "b"},
            {TokenType::SEMICOLON,{}},
            {TokenType::RBRACE,   {}},
        }
    );
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

TEST(LexerEdgeCases, ConsecutiveOperatorsNoWhitespace) {
    assertTokenSequence("+-*/", {
        {TokenType::PLUS,  {}},
        {TokenType::MINUS, {}},
        {TokenType::STAR,  {}},
        {TokenType::SLASH, {}},
    });
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
