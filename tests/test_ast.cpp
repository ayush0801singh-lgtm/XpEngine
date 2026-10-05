// Unit tests for AST nodes and the ASTVisitor.
//
// These tests prove that the AST cleans up its own memory correctly without
// any manual deletes, and that the visitor visits every node type properly.

#include "expression/ast/ASTNode.hpp"
#include "expression/ast/ASTPrinter.hpp"
#include "expression/ast/ASTVisitor.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

namespace {

using namespace expr;

// ---------------------------------------------------------------------------
// Memory Ownership / Lifetime Tests
// ---------------------------------------------------------------------------

// A mock node that increments a counter on destruction, used to prove that
// destroying a parent node correctly cascades to its children.
class InstanceCountingNode final : public ASTNode {
public:
    explicit InstanceCountingNode(int& counter)
        : ASTNode(std::nullopt)
        , counter_(counter)
    {
    }

    ~InstanceCountingNode() override
    {
        ++counter_;
    }

    void accept(ASTVisitor&) const override {}

private:
    int& counter_;
};

TEST(ASTOwnership, UnaryNodeDestroysOperand)
{
    int destroyed = 0;
    {
        UnaryOpNode op(UnaryOperator::Minus,
                       std::make_unique<InstanceCountingNode>(destroyed));
        EXPECT_EQ(destroyed, 0);
    }
    EXPECT_EQ(destroyed, 1);
}

TEST(ASTOwnership, BinaryNodeDestroysBothOperands)
{
    int destroyed = 0;
    {
        BinaryOpNode op(BinaryOperator::Add,
                        std::make_unique<InstanceCountingNode>(destroyed),
                        std::make_unique<InstanceCountingNode>(destroyed));
        EXPECT_EQ(destroyed, 0);
    }
    EXPECT_EQ(destroyed, 2);
}

TEST(ASTOwnership, FunctionCallDestroysAllArguments)
{
    int destroyed = 0;
    {
        std::vector<ASTNodePtr> args;
        args.push_back(std::make_unique<InstanceCountingNode>(destroyed));
        args.push_back(std::make_unique<InstanceCountingNode>(destroyed));
        args.push_back(std::make_unique<InstanceCountingNode>(destroyed));

        FunctionCallNode call("max", std::move(args));
        EXPECT_EQ(destroyed, 0);
    }
    EXPECT_EQ(destroyed, 3);
}

TEST(ASTOwnership, DeepTreeIsDestroyedAutomatically)
{
    int destroyed = 0;
    {
        // - (a + b)
        auto add = std::make_unique<BinaryOpNode>(
            BinaryOperator::Add,
            std::make_unique<InstanceCountingNode>(destroyed),
            std::make_unique<InstanceCountingNode>(destroyed));

        UnaryOpNode root(UnaryOperator::Minus, std::move(add));
    }
    EXPECT_EQ(destroyed, 2);
}

// ---------------------------------------------------------------------------
// Class Invariants
// ---------------------------------------------------------------------------

TEST(ASTInvariants, UnaryNodeRejectsNullOperand)
{
    EXPECT_THROW(UnaryOpNode(UnaryOperator::Minus, nullptr), std::invalid_argument);
}

TEST(ASTInvariants, BinaryNodeRejectsNullOperands)
{
    auto makeNumber = [] { return std::make_unique<NumberNode>(1.0); };

    EXPECT_THROW(BinaryOpNode(BinaryOperator::Add, nullptr, makeNumber()),
                 std::invalid_argument);
    EXPECT_THROW(BinaryOpNode(BinaryOperator::Add, makeNumber(), nullptr),
                 std::invalid_argument);
}

TEST(ASTInvariants, FunctionCallRejectsNullArguments)
{
    std::vector<ASTNodePtr> args;
    args.push_back(std::make_unique<NumberNode>(1.0));
    args.push_back(nullptr);

    EXPECT_THROW(FunctionCallNode("f", std::move(args)), std::invalid_argument);
}

// ---------------------------------------------------------------------------
// Visitor / Printer Tests
// ---------------------------------------------------------------------------

TEST(ASTPrinter, FormatsNumber)
{
    EXPECT_EQ(formatSExpression(NumberNode(42.5)), "42.5");
}

TEST(ASTPrinter, FormatsVariable)
{
    EXPECT_EQ(formatSExpression(VariableNode("x")), "x");
}

TEST(ASTPrinter, FormatsUnaryOperator)
{
    UnaryOpNode op(UnaryOperator::Minus, std::make_unique<VariableNode>("y"));
    EXPECT_EQ(formatSExpression(op), "(- y)");
}

TEST(ASTPrinter, FormatsBinaryOperator)
{
    BinaryOpNode op(BinaryOperator::Add,
                    std::make_unique<NumberNode>(1),
                    std::make_unique<NumberNode>(2));
    EXPECT_EQ(formatSExpression(op), "(+ 1 2)");
}

TEST(ASTPrinter, FormatsFunctionCall)
{
    std::vector<ASTNodePtr> args;
    args.push_back(std::make_unique<VariableNode>("a"));
    args.push_back(std::make_unique<VariableNode>("b"));
    FunctionCallNode call("max", std::move(args));

    EXPECT_EQ(formatSExpression(call), "(max a b)");
}

TEST(ASTPrinter, FormatsDeepTree)
{
    // max(a, -b) * 2
    std::vector<ASTNodePtr> args;
    args.push_back(std::make_unique<VariableNode>("a"));
    args.push_back(std::make_unique<UnaryOpNode>(UnaryOperator::Minus,
                                                 std::make_unique<VariableNode>("b")));

    BinaryOpNode root(BinaryOperator::Multiply,
                      std::make_unique<FunctionCallNode>("max", std::move(args)),
                      std::make_unique<NumberNode>(2));

    EXPECT_EQ(formatSExpression(root), "(* (max a (- b)) 2)");
}

} // namespace
