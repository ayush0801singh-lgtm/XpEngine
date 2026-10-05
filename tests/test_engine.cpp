// Unit tests for the Engine facade.
//
// These tests verify that the engine catches exceptions correctly and manages
// the environment appropriately across multiple evaluation calls.

#include "expression/Engine.hpp"
#include "expression/Result.hpp"

#include <gtest/gtest.h>

#include <string>

namespace {

using namespace expr;

TEST(Engine, SuccessfulEvaluationReturnsValue)
{
    Engine engine;
    const Result result = engine.evaluate("2 + 3 * 4");
    
    EXPECT_FALSE(result.hasError());
    EXPECT_DOUBLE_EQ(result.value(), 14.0);
}

TEST(Engine, CatchLexError)
{
    Engine engine;
    const Result result = engine.evaluate("1.2.3");
    
    ASSERT_TRUE(result.hasError());
    EXPECT_NE(result.error().message.find("Lexical"), std::string::npos);
    EXPECT_EQ(result.error().position, 3U);
}

TEST(Engine, CatchParseError)
{
    Engine engine;
    const Result result = engine.evaluate("1 + * 2");
    
    ASSERT_TRUE(result.hasError());
    EXPECT_NE(result.error().message.find("Parse"), std::string::npos);
    EXPECT_EQ(result.error().position, 4U);
}

TEST(Engine, CatchEvaluationError)
{
    Engine engine;
    const Result result = engine.evaluate("5 / 0");
    
    ASSERT_TRUE(result.hasError());
    EXPECT_NE(result.error().message.find("Evaluation"), std::string::npos);
    // 5 / 0, division sign is at position 2
    EXPECT_EQ(result.error().position, 2U);
}

TEST(Engine, StateIsPersistedAcrossCalls)
{
    Engine engine;
    
    engine.environment().set("x", 10.0);
    
    const Result r1 = engine.evaluate("x * 2");
    ASSERT_FALSE(r1.hasError());
    EXPECT_DOUBLE_EQ(r1.value(), 20.0);
    
    engine.environment().set("y", r1.value());
    
    const Result r2 = engine.evaluate("x + y");
    ASSERT_FALSE(r2.hasError());
    EXPECT_DOUBLE_EQ(r2.value(), 30.0);
}

} // namespace
