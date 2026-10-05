// Unit tests for the lexer (expression/lexer/Lexer.hpp).

#include "expression/errors/Errors.hpp"
#include "expression/lexer/Lexer.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <string>
#include <vector>

namespace {

using expr::LexError;
using expr::Token;
using expr::TokenType;
using expr::tokenize;

std::vector<TokenType> typesOf(const std::vector<Token>& tokens)
{
    std::vector<TokenType> types;
    types.reserve(tokens.size());
    for (const Token& token : tokens) {
        types.push_back(token.type);
    }
    return types;
}

/// Lexes `source` expecting exactly one Number token followed by End.
double lexSingleNumber(const std::string& source)
{
    const auto tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 2U) << "source: " << source;
    EXPECT_EQ(tokens.at(0).type, TokenType::Number) << "source: " << source;
    EXPECT_EQ(tokens.at(1).type, TokenType::End) << "source: " << source;
    return tokens.at(0).number;
}

/// Asserts that lexing `source` throws LexError at `expectedPosition` and
/// that the message contains `expectedFragment`.
void expectLexError(const std::string& source,
                    std::size_t expectedPosition,
                    const std::string& expectedFragment)
{
    try {
        (void)tokenize(source);
        ADD_FAILURE() << "expected LexError for: " << source;
    } catch (const LexError& error) {
        ASSERT_TRUE(error.position().has_value());
        EXPECT_EQ(*error.position(), expectedPosition) << "source: " << source;
        EXPECT_NE(std::string(error.what()).find(expectedFragment), std::string::npos)
            << "message was: " << error.what();
    }
}

// ---------------------------------------------------------------------------
// Empty input / whitespace
// ---------------------------------------------------------------------------

TEST(Lexer, EmptyInputProducesOnlyEnd)
{
    const auto tokens = tokenize("");
    ASSERT_EQ(tokens.size(), 1U);
    EXPECT_EQ(tokens[0].type, TokenType::End);
    EXPECT_EQ(tokens[0].position, 0U);
}

TEST(Lexer, WhitespaceOnlyProducesOnlyEnd)
{
    const auto tokens = tokenize(" \t\r\n ");
    ASSERT_EQ(tokens.size(), 1U);
    EXPECT_EQ(tokens[0].type, TokenType::End);
    EXPECT_EQ(tokens[0].position, 5U);
}

TEST(Lexer, AllWhitespaceKindsSeparateTokens)
{
    const auto tokens = tokenize("1\t+\n2\r*\f3\v");
    EXPECT_EQ(typesOf(tokens),
              (std::vector<TokenType>{TokenType::Number, TokenType::Plus,
                                      TokenType::Number, TokenType::Star,
                                      TokenType::Number, TokenType::End}));
}

TEST(Lexer, NextKeepsReturningEndAfterInputIsExhausted)
{
    expr::Lexer lexer("7");
    EXPECT_EQ(lexer.next().type, TokenType::Number);
    EXPECT_EQ(lexer.next().type, TokenType::End);
    EXPECT_EQ(lexer.next().type, TokenType::End);
    EXPECT_EQ(lexer.next().type, TokenType::End);
}

// ---------------------------------------------------------------------------
// Numbers
// ---------------------------------------------------------------------------

TEST(Lexer, Integers)
{
    EXPECT_DOUBLE_EQ(lexSingleNumber("0"), 0.0);
    EXPECT_DOUBLE_EQ(lexSingleNumber("42"), 42.0);
    EXPECT_DOUBLE_EQ(lexSingleNumber("007"), 7.0);
}

TEST(Lexer, DecimalNumbers)
{
    EXPECT_DOUBLE_EQ(lexSingleNumber("3.14"), 3.14);
    EXPECT_DOUBLE_EQ(lexSingleNumber("0.001"), 0.001);
    EXPECT_DOUBLE_EQ(lexSingleNumber(".5"), 0.5);
}

TEST(Lexer, ScientificNotation)
{
    EXPECT_DOUBLE_EQ(lexSingleNumber("1e3"), 1000.0);
    EXPECT_DOUBLE_EQ(lexSingleNumber("2.5E-2"), 0.025);
    EXPECT_DOUBLE_EQ(lexSingleNumber("1e+2"), 100.0);
    EXPECT_DOUBLE_EQ(lexSingleNumber(".5e1"), 5.0);
}

TEST(Lexer, VeryLongIntegerIsRepresentedApproximately)
{
    const double value = lexSingleNumber("123456789012345678901234567890");
    EXPECT_DOUBLE_EQ(value, 1.2345678901234568e29);
}

TEST(Lexer, NumberTokenKeepsOriginalLexeme)
{
    const auto tokens = tokenize("2.50");
    EXPECT_EQ(tokens[0].lexeme, "2.50");
}

TEST(Lexer, SignIsNeverPartOfANumber)
{
    // "-3" is two tokens; the parser decides that this '-' is unary.
    EXPECT_EQ(typesOf(tokenize("-3")),
              (std::vector<TokenType>{TokenType::Minus, TokenType::Number,
                                      TokenType::End}));

    // "2-3" must be a binary minus, not "2" followed by the literal "-3".
    const auto tokens = tokenize("2-3");
    EXPECT_EQ(typesOf(tokens),
              (std::vector<TokenType>{TokenType::Number, TokenType::Minus,
                                      TokenType::Number, TokenType::End}));
    EXPECT_DOUBLE_EQ(tokens[2].number, 3.0);
}

// ---------------------------------------------------------------------------
// Identifiers
// ---------------------------------------------------------------------------

TEST(Lexer, Identifiers)
{
    for (const std::string name : {"x", "sin", "_tmp", "var_2", "CamelCase", "a1b2"}) {
        const auto tokens = tokenize(name);
        ASSERT_EQ(tokens.size(), 2U) << name;
        EXPECT_EQ(tokens[0].type, TokenType::Identifier) << name;
        EXPECT_EQ(tokens[0].lexeme, name);
    }
}

TEST(Lexer, IdentifierFollowedByParenIsStillAnIdentifier)
{
    // Function names are not keywords; the parser recognises a call by the
    // '(' that follows the identifier.
    EXPECT_EQ(typesOf(tokenize("max(a,b)")),
              (std::vector<TokenType>{TokenType::Identifier, TokenType::LeftParen,
                                      TokenType::Identifier, TokenType::Comma,
                                      TokenType::Identifier, TokenType::RightParen,
                                      TokenType::End}));
}

// ---------------------------------------------------------------------------
// Operators and punctuation
// ---------------------------------------------------------------------------

TEST(Lexer, AllOperatorsAndPunctuation)
{
    const auto tokens = tokenize("+-*/%^(),");
    EXPECT_EQ(typesOf(tokens),
              (std::vector<TokenType>{TokenType::Plus, TokenType::Minus,
                                      TokenType::Star, TokenType::Slash,
                                      TokenType::Percent, TokenType::Caret,
                                      TokenType::LeftParen, TokenType::RightParen,
                                      TokenType::Comma, TokenType::End}));
    EXPECT_EQ(tokens[5].lexeme, "^");
}

TEST(Lexer, ExpressionWithoutWhitespace)
{
    EXPECT_EQ(typesOf(tokenize("3+4*2")),
              (std::vector<TokenType>{TokenType::Number, TokenType::Plus,
                                      TokenType::Number, TokenType::Star,
                                      TokenType::Number, TokenType::End}));
}

TEST(Lexer, NestedFunctionExpression)
{
    EXPECT_EQ(typesOf(tokenize("sqrt(x^2 + y^2)")),
              (std::vector<TokenType>{TokenType::Identifier, TokenType::LeftParen,
                                      TokenType::Identifier, TokenType::Caret,
                                      TokenType::Number, TokenType::Plus,
                                      TokenType::Identifier, TokenType::Caret,
                                      TokenType::Number, TokenType::RightParen,
                                      TokenType::End}));
}

// ---------------------------------------------------------------------------
// Positions
// ---------------------------------------------------------------------------

TEST(Lexer, TokensRecordTheirStartPosition)
{
    //                      0123456789
    const auto tokens = tokenize("  12 + xy");
    ASSERT_EQ(tokens.size(), 4U);
    EXPECT_EQ(tokens[0].position, 2U); // 12
    EXPECT_EQ(tokens[1].position, 5U); // +
    EXPECT_EQ(tokens[2].position, 7U); // xy
    EXPECT_EQ(tokens[3].position, 9U); // End == source length
}

// ---------------------------------------------------------------------------
// Errors
// ---------------------------------------------------------------------------

TEST(LexerErrors, InvalidCharacter)
{
    expectLexError("3 $ 4", 2, "unexpected character '$'");
    expectLexError("#", 0, "'#'");
    expectLexError("a = 1", 2, "'='"); // assignment is not part of the expression grammar
}

TEST(LexerErrors, LoneDotIsInvalid)
{
    expectLexError("3 + .", 4, "unexpected character '.'");
}

TEST(LexerErrors, NonAsciiByteIsReportedAsHex)
{
    expectLexError("x + \xC3\xA9", 4, "byte 0xC3");
}

TEST(LexerErrors, ErrorMessageUsesOneBasedColumn)
{
    try {
        (void)tokenize("3 $ 4");
        FAIL() << "expected LexError";
    } catch (const LexError& error) {
        EXPECT_EQ(std::string(error.what()),
                  "Lexical error at column 3: unexpected character '$'");
    }
}

TEST(LexerErrors, LexErrorIsAnExpressionError)
{
    EXPECT_THROW((void)tokenize("@"), expr::ExpressionError);
}

TEST(LexerErrors, MalformedNumbers)
{
    expectLexError("1.2.3", 3, "malformed number '1.2.3'");
    expectLexError("3.", 2, "expected digit after decimal point");
    expectLexError("3.x", 2, "malformed number '3.x'");
    expectLexError("1e", 2, "exponent has no digits");
    expectLexError("1e+", 3, "exponent has no digits");
    expectLexError("2 * 1E-", 7, "exponent has no digits");
    expectLexError("12abc", 2, "malformed number '12abc'");
    expectLexError("1e5x", 3, "unexpected character 'x' in number");
}

TEST(LexerErrors, NumberOutOfRange)
{
    expectLexError("1e999", 0, "out of range");
}

} // namespace
