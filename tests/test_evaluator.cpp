// Unit tests for evaluation (expression/evaluator/Evaluator.hpp).
//
// These tests execute fully parsed ASTs to prove the math is right.

#include "expression/evaluator/Environment.hpp"
#include "expression/evaluator/EvaluationError.hpp"
#include "expression/evaluator/Evaluator.hpp"
#include "expression/parser/Parser.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <string>

namespace {

using namespace expr;

double eval(const std::string& source, const Environment& env = Environment())
{
    return evaluate(*parse(source), env);
}

void expectEvalError(const std::string& source, const std::string& expectedFragment)
{
    try {
        (void)eval(source);
        ADD_FAILURE() << "expected EvaluationError for: " << source;
    } catch (const EvaluationError& error) {
        EXPECT_NE(std::string(error.what()).find(expectedFragment), std::string::npos)
            << "source: " << source << "\nmessage: " << error.what();
    }
}

void expectEvalError(const std::string& source,
                     const Environment& env,
                     const std::string& expectedFragment)
{
    try {
        (void)eval(source, env);
        ADD_FAILURE() << "expected EvaluationError for: " << source;
    } catch (const EvaluationError& error) {
        EXPECT_NE(std::string(error.what()).find(expectedFragment), std::string::npos)
            << "source: " << source << "\nmessage: " << error.what();
    }
}

// ---------------------------------------------------------------------------
// Basic Arithmetic
// ---------------------------------------------------------------------------

TEST(Evaluator, Numbers)
{
    EXPECT_DOUBLE_EQ(eval("42"), 42.0);
    EXPECT_DOUBLE_EQ(eval("3.14"), 3.14);
}

TEST(Evaluator, Addition)
{
    EXPECT_DOUBLE_EQ(eval("2 + 3"), 5.0);
    EXPECT_DOUBLE_EQ(eval("2 + -3"), -1.0);
}

TEST(Evaluator, Subtraction)
{
    EXPECT_DOUBLE_EQ(eval("10 - 4"), 6.0);
    EXPECT_DOUBLE_EQ(eval("10 - 4 - 3"), 3.0);
}

TEST(Evaluator, Multiplication)
{
    EXPECT_DOUBLE_EQ(eval("3 * 5"), 15.0);
    EXPECT_DOUBLE_EQ(eval("2 * 3 * 4"), 24.0);
}

TEST(Evaluator, Division)
{
    EXPECT_DOUBLE_EQ(eval("10 / 2"), 5.0);
    EXPECT_DOUBLE_EQ(eval("8 / 4 / 2"), 1.0);
}

TEST(Evaluator, Modulo)
{
    EXPECT_DOUBLE_EQ(eval("10 % 3"), 1.0);
    EXPECT_DOUBLE_EQ(eval("10.5 % 3"), 1.5);
}

TEST(Evaluator, Power)
{
    EXPECT_DOUBLE_EQ(eval("2 ^ 3"), 8.0);
    EXPECT_DOUBLE_EQ(eval("2 ^ 3 ^ 2"), 512.0); // right-associative: 2^(3^2) = 2^9
    EXPECT_DOUBLE_EQ(eval("4 ^ 0.5"), 2.0);
}

TEST(Evaluator, Unary)
{
    EXPECT_DOUBLE_EQ(eval("-5"), -5.0);
    EXPECT_DOUBLE_EQ(eval("+5"), 5.0);
    EXPECT_DOUBLE_EQ(eval("-(2 + 3)"), -5.0);
    EXPECT_DOUBLE_EQ(eval("--3"), 3.0);
}

// ---------------------------------------------------------------------------
// Operator Precedence (Evaluation)
// ---------------------------------------------------------------------------

TEST(EvaluatorPrecedence, MixedOperations)
{
    EXPECT_DOUBLE_EQ(eval("2 + 3 * 4"), 14.0);
    EXPECT_DOUBLE_EQ(eval("(2 + 3) * 4"), 20.0);
    EXPECT_DOUBLE_EQ(eval("-2 ^ 2"), -4.0);
}

// ---------------------------------------------------------------------------
// Variables
// ---------------------------------------------------------------------------

TEST(EvaluatorVariables, Lookup)
{
    Environment env;
    env.set("x", 10.0);
    env.set("y", 2.5);

    EXPECT_DOUBLE_EQ(eval("x", env), 10.0);
    EXPECT_DOUBLE_EQ(eval("x + y", env), 12.5);
    EXPECT_DOUBLE_EQ(eval("x * y + 2", env), 27.0);
}

TEST(EvaluatorVariables, UndefinedVariable)
{
    expectEvalError("x + 5", Environment(), "undefined variable 'x'");
}

// ---------------------------------------------------------------------------
// Functions
// ---------------------------------------------------------------------------

TEST(EvaluatorFunctions, Builtins)
{
    EXPECT_DOUBLE_EQ(eval("sin(0)"), 0.0);
    EXPECT_DOUBLE_EQ(eval("cos(0)"), 1.0);
    EXPECT_DOUBLE_EQ(eval("tan(0)"), 0.0);
    EXPECT_DOUBLE_EQ(eval("sqrt(16)"), 4.0);
    EXPECT_DOUBLE_EQ(eval("exp(0)"), 1.0);
    EXPECT_DOUBLE_EQ(eval("log(1)"), 0.0);
    EXPECT_DOUBLE_EQ(eval("abs(-4.5)"), 4.5);
    EXPECT_DOUBLE_EQ(eval("min(3, 7)"), 3.0);
    EXPECT_DOUBLE_EQ(eval("max(3, 7)"), 7.0);
}

TEST(EvaluatorFunctions, NestedExpressions)
{
    Environment env;
    env.set("x", 3.0);
    env.set("y", 4.0);

    EXPECT_DOUBLE_EQ(eval("sqrt(x^2 + y^2)", env), 5.0);
}

// ---------------------------------------------------------------------------
// Evaluation Errors
// ---------------------------------------------------------------------------

TEST(EvaluatorErrors, DivisionByZero)
{
    expectEvalError("5 / 0", "division by zero");
    expectEvalError("5 / (2 - 2)", "division by zero");
}

TEST(EvaluatorErrors, ModuloByZero)
{
    expectEvalError("5 % 0", "modulo by zero");
}

TEST(EvaluatorErrors, UnknownFunction)
{
    expectEvalError("unknownFunction(5)", "unknown function 'unknownFunction'");
}

TEST(EvaluatorErrors, IncorrectArgumentCount)
{
    expectEvalError("sin(1, 2)", "expects 1 argument(s), got 2");
    expectEvalError("max(1)", "expects 2 argument(s), got 1");
    expectEvalError("max(1, 2, 3)", "expects 2 argument(s), got 3");
}

TEST(EvaluatorErrors, MathDomainErrors)
{
    expectEvalError("sqrt(-1)", "sqrt of negative number");
    expectEvalError("log(0)", "log of non-positive number");
    expectEvalError("log(-1)", "log of non-positive number");
}

TEST(EvaluatorErrors, ErrorIncludesPosition)
{
    try {
        (void)eval("1 + 5 / 0");
        FAIL() << "expected EvaluationError";
    } catch (const EvaluationError& error) {
        // The division operator is at index 6 in "1 + 5 / 0"
        EXPECT_EQ(*error.position(), 6U);
        EXPECT_EQ(std::string(error.what()), "Evaluation error at column 7: division by zero");
    }
}

} // namespace
