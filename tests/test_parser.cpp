// Unit tests for the parser (expression/parser/Parser.hpp).
//
// Trees are compared as S-expressions, which makes precedence and
// associativity visible at a glance:
//
//   "3 + 4 * 2"  ->  (+ 3 (* 4 2))
//   "-x"         ->  (- x)          a unary operator has one child
//   "max(a, b)"  ->  (max a b)

#include "expression/ast/ASTPrinter.hpp"
#include "expression/errors/Errors.hpp"
#include "expression/lexer/Lexer.hpp"
#include "expression/parser/Parser.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace {

using namespace expr;

std::string parsed(const std::string& source)
{
    return formatSExpression(*parse(source));
}

void expectParseError(const std::string& source,
                      std::size_t expectedPosition,
                      const std::string& expectedFragment)
{
    try {
        (void)parse(source);
        ADD_FAILURE() << "expected ParseError for: " << source;
    } catch (const ParseError& error) {
        ASSERT_TRUE(error.position().has_value());
        EXPECT_EQ(*error.position(), expectedPosition) << "source: " << source
                                                       << "\nmessage: " << error.what();
        EXPECT_NE(std::string(error.what()).find(expectedFragment), std::string::npos)
            << "source: " << source << "\nmessage: " << error.what();
    }
}

// ---------------------------------------------------------------------------
// Atoms
// ---------------------------------------------------------------------------

TEST(Parser, SingleNumber)
{
    EXPECT_EQ(parsed("42"), "42");
    EXPECT_EQ(parsed("2.5"), "2.5");
}

TEST(Parser, SingleVariable)
{
    EXPECT_EQ(parsed("x"), "x");
}

TEST(Parser, RedundantParenthesesLeaveNoTraceInTree)
{
    EXPECT_EQ(parsed("((((5))))"), "5");
    EXPECT_EQ(parsed("(x)"), "x");
}

// ---------------------------------------------------------------------------
// Precedence
// ---------------------------------------------------------------------------

TEST(ParserPrecedence, MultiplicationBindsTighterThanAddition)
{
    EXPECT_EQ(parsed("3 + 4 * 2"), "(+ 3 (* 4 2))");
    EXPECT_EQ(parsed("3 * 4 + 2"), "(+ (* 3 4) 2)");
    EXPECT_EQ(parsed("2 + 3 * 4 - 5"), "(- (+ 2 (* 3 4)) 5)");
}

TEST(ParserPrecedence, ParenthesesOverridePrecedence)
{
    EXPECT_EQ(parsed("(3 + 4) * 2"), "(* (+ 3 4) 2)");
    EXPECT_EQ(parsed("2 * (3 + (4 - 1))"), "(* 2 (+ 3 (- 4 1)))");
}

TEST(ParserPrecedence, ModuloHasMultiplicativePrecedence)
{
    EXPECT_EQ(parsed("1 + 7 % 3"), "(+ 1 (% 7 3))");
}

TEST(ParserPrecedence, PowerBindsTighterThanMultiplication)
{
    EXPECT_EQ(parsed("2 * 3 ^ 2"), "(* 2 (^ 3 2))");
}

TEST(ParserPrecedence, Polynomial)
{
    EXPECT_EQ(parsed("x^2 + 2*x + 1"), "(+ (+ (^ x 2) (* 2 x)) 1)");
}

// ---------------------------------------------------------------------------
// Associativity
// ---------------------------------------------------------------------------

TEST(ParserAssociativity, SubtractionIsLeftAssociative)
{
    EXPECT_EQ(parsed("10 - 4 - 3"), "(- (- 10 4) 3)");
}

TEST(ParserAssociativity, DivisionAndModuloAreLeftAssociative)
{
    EXPECT_EQ(parsed("8 / 4 / 2"), "(/ (/ 8 4) 2)");
    EXPECT_EQ(parsed("a % b * c"), "(* (% a b) c)");
}

TEST(ParserAssociativity, PowerIsRightAssociative)
{
    EXPECT_EQ(parsed("2 ^ 3 ^ 2"), "(^ 2 (^ 3 2))");
}

// ---------------------------------------------------------------------------
// Unary operators
// ---------------------------------------------------------------------------

TEST(ParserUnary, LeadingMinusAndPlus)
{
    EXPECT_EQ(parsed("-5"), "(- 5)");
    EXPECT_EQ(parsed("+5"), "(+ 5)");
    EXPECT_EQ(parsed("-(2 + 3)"), "(- (+ 2 3))");
}

TEST(ParserUnary, RepeatedUnaryOperators)
{
    EXPECT_EQ(parsed("--3"), "(- (- 3))");
    EXPECT_EQ(parsed("-+-3"), "(- (+ (- 3)))");
}

TEST(ParserUnary, UnaryAfterBinaryOperator)
{
    EXPECT_EQ(parsed("3 - -4"), "(- 3 (- 4))");
    EXPECT_EQ(parsed("3 * -4"), "(* 3 (- 4))");
    EXPECT_EQ(parsed("2 ^ -3"), "(^ 2 (- 3))");
}

TEST(ParserUnary, UnaryMinusBindsLooserThanPower)
{
    // Mathematical convention: -2^2 = -(2^2) = -4
    EXPECT_EQ(parsed("-2 ^ 2"), "(- (^ 2 2))");
}

TEST(ParserUnary, UnaryMinusBindsTighterThanMultiplication)
{
    EXPECT_EQ(parsed("-x * y"), "(* (- x) y)");
    EXPECT_EQ(parsed("-x + y"), "(+ (- x) y)");
}

TEST(ParserUnary, SpecExample)
{
    EXPECT_EQ(parsed("-3 + 5 * (x - 2)"), "(+ (- 3) (* 5 (- x 2)))");
}

// ---------------------------------------------------------------------------
// Function calls
// ---------------------------------------------------------------------------

TEST(ParserFunctions, SingleArgument)
{
    EXPECT_EQ(parsed("sin(x)"), "(sin x)");
    EXPECT_EQ(parsed("sin(x) + cos(y)"), "(+ (sin x) (cos y))");
}

TEST(ParserFunctions, MultipleArguments)
{
    EXPECT_EQ(parsed("max(a, b) * 2"), "(* (max a b) 2)");
    EXPECT_EQ(parsed("f(1, 2, 3, 4)"), "(f 1 2 3 4)");
}

TEST(ParserFunctions, ZeroArguments)
{
    EXPECT_EQ(parsed("f()"), "(f)");
    EXPECT_EQ(parsed("f() + 1"), "(+ (f) 1)");
}

TEST(ParserFunctions, ArgumentsAreFullExpressions)
{
    EXPECT_EQ(parsed("max(1 + 2, 3 * 4)"), "(max (+ 1 2) (* 3 4))");
    EXPECT_EQ(parsed("f(-x)"), "(f (- x))");
    EXPECT_EQ(parsed("f((1 + 2) * 3)"), "(f (* (+ 1 2) 3))");
}

TEST(ParserFunctions, NestedCalls)
{
    EXPECT_EQ(parsed("max(1, max(2, 3))"), "(max 1 (max 2 3))");
    EXPECT_EQ(parsed("sqrt(x^2 + y^2)"), "(sqrt (+ (^ x 2) (^ y 2)))");
    EXPECT_EQ(parsed("sin(cos(tan(x)))"), "(sin (cos (tan x)))");
}

TEST(ParserFunctions, UnaryMinusAppliedToCall)
{
    EXPECT_EQ(parsed("-abs(x)"), "(- (abs x))");
}

// ---------------------------------------------------------------------------
// Source positions
// ---------------------------------------------------------------------------

TEST(Parser, NodesRecordTokenPositions)
{
    //                          0123456789
    const ASTNodePtr root = parse("1 + f(x)");
    const auto& add = dynamic_cast<const BinaryOpNode&>(*root);
    EXPECT_EQ(add.position(), 2U);
    EXPECT_EQ(add.left().position(), 0U);
    EXPECT_EQ(add.right().position(), 4U); // call is located at its name
    const auto& call = dynamic_cast<const FunctionCallNode&>(add.right());
    EXPECT_EQ(call.argument(0).position(), 6U);
}

TEST(Parser, ParseFromTokenVector)
{
    const auto root = parse(tokenize("1 + 2"));
    EXPECT_EQ(formatSExpression(*root), "(+ 1 2)");
}

TEST(Parser, TokenVectorWithoutEndIsRejected)
{
    EXPECT_THROW((void)parse(std::vector<Token>{}), std::invalid_argument);
    std::vector<Token> tokens = tokenize("1");
    tokens.pop_back(); // drop End
    EXPECT_THROW((void)parse(tokens), std::invalid_argument);
}

// ---------------------------------------------------------------------------
// Errors
// ---------------------------------------------------------------------------

TEST(ParserErrors, EmptyInput)
{
    expectParseError("", 0, "empty expression");
    expectParseError("   ", 3, "empty expression");
}

TEST(ParserErrors, MissingOperandAtEnd)
{
    expectParseError("3 +", 3, "expected expression after '+', found end of input");
    expectParseError("-", 1, "expected expression after '-'");
}

TEST(ParserErrors, MissingOperandBetweenOperators)
{
    expectParseError("3 + * 4", 4, "expected expression after '+', found '*'");
    expectParseError("* 3", 0, "expected expression, found '*'");
    expectParseError("2 ^ / 3", 4, "found '/'");
}

TEST(ParserErrors, UnmatchedOpenParenthesis)
{
    expectParseError("((3 + 4)", 0, "unmatched '('");
    expectParseError("2 * (3 + 4", 4, "unmatched '('");
}

TEST(ParserErrors, UnmatchedCloseParenthesis)
{
    expectParseError("(3 + 4))", 7, "unmatched ')'");
    expectParseError("3)", 1, "unmatched ')'");
}

TEST(ParserErrors, EmptyParentheses)
{
    expectParseError("()", 1, "expected expression after '(', found ')'");
}

TEST(ParserErrors, MissingOperator)
{
    expectParseError("3 4", 2, "unexpected number '4'");
    expectParseError("2 x", 2, "unexpected identifier 'x'");
    expectParseError("2 (3)", 2, "unexpected '('");
    expectParseError("f(1)(2)", 4, "unexpected '('");
}

TEST(ParserErrors, BadFunctionArgumentLists)
{
    expectParseError("max(1,)", 6, "expected expression after ','");
    expectParseError("max(,1)", 4, "expected expression after '(', found ','");
    expectParseError("max(1, 2", 0, "missing ')' to close call to 'max'");
}

TEST(ParserErrors, CommaOutsideFunctionCall)
{
    expectParseError("1, 2", 1, "',' is only allowed between function arguments");
    expectParseError("(1, 2)", 2, "',' is only allowed between function arguments");
}

TEST(ParserErrors, MessageFormat)
{
    try {
        (void)parse("3 +");
        FAIL() << "expected ParseError";
    } catch (const ParseError& error) {
        EXPECT_EQ(std::string(error.what()),
                  "Parse error at column 4: expected expression after '+', "
                  "found end of input");
    }
}

TEST(ParserErrors, LexErrorsPropagateThroughParse)
{
    EXPECT_THROW((void)parse("3 + $"), LexError);
}

TEST(ParserErrors, AllErrorsShareACommonBase)
{
    EXPECT_THROW((void)parse("3 +"), ExpressionError);
    EXPECT_THROW((void)parse("3 $"), ExpressionError);
}

} // namespace
