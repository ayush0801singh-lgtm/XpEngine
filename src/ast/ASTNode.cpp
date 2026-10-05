#include "expression/ast/ASTNode.hpp"
#include "expression/ast/ASTVisitor.hpp"

#include <stdexcept>
#include <utility>

namespace expr {

namespace {

ASTNodePtr requireNonNull(ASTNodePtr node, const char* what)
{
    if (!node) {
        throw std::invalid_argument(std::string("AST node requires a non-null ") + what);
    }
    return node;
}

} // namespace

NumberNode::NumberNode(double value, std::optional<std::size_t> position) noexcept
    : ASTNode(position)
    , value_(value)
{
}

void NumberNode::accept(ASTVisitor& visitor) const
{
    visitor.visit(*this);
}

VariableNode::VariableNode(std::string name, std::optional<std::size_t> position)
    : ASTNode(position)
    , name_(std::move(name))
{
}

void VariableNode::accept(ASTVisitor& visitor) const
{
    visitor.visit(*this);
}

UnaryOpNode::UnaryOpNode(UnaryOperator op,
                         ASTNodePtr operand,
                         std::optional<std::size_t> position)
    : ASTNode(position)
    , op_(op)
    , operand_(requireNonNull(std::move(operand), "operand"))
{
}

void UnaryOpNode::accept(ASTVisitor& visitor) const
{
    visitor.visit(*this);
}

BinaryOpNode::BinaryOpNode(BinaryOperator op,
                           ASTNodePtr left,
                           ASTNodePtr right,
                           std::optional<std::size_t> position)
    : ASTNode(position)
    , op_(op)
    , left_(requireNonNull(std::move(left), "left operand"))
    , right_(requireNonNull(std::move(right), "right operand"))
{
}

void BinaryOpNode::accept(ASTVisitor& visitor) const
{
    visitor.visit(*this);
}

FunctionCallNode::FunctionCallNode(std::string name,
                                   std::vector<ASTNodePtr> arguments,
                                   std::optional<std::size_t> position)
    : ASTNode(position)
    , name_(std::move(name))
    , arguments_(std::move(arguments))
{
    for (const ASTNodePtr& argument : arguments_) {
        if (!argument) {
            throw std::invalid_argument("AST node requires non-null function arguments");
        }
    }
}

void FunctionCallNode::accept(ASTVisitor& visitor) const
{
    visitor.visit(*this);
}

} // namespace expr
