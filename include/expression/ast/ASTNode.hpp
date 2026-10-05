#pragma once

#include "expression/ast/Operators.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace expr {

class ASTNode;

/// Exclusive ownership of a subtree. Every node is owned by exactly one
/// parent (or by the caller, for the root), so destroying the root releases
/// the whole tree automatically - no manual delete anywhere.
using ASTNodePtr = std::unique_ptr<ASTNode>;

/// Abstract base of all AST nodes.
///
/// Nodes are immutable after construction: all accessors are const and there
/// are no setters. Nodes are also non-copyable and non-movable; they are
/// always handled through ASTNodePtr, which prevents object slicing and makes
/// ownership explicit at every API boundary.
class ASTNode {
public:
    virtual ~ASTNode() = default;

    ASTNode(const ASTNode&) = delete;
    ASTNode& operator=(const ASTNode&) = delete;
    ASTNode(ASTNode&&) = delete;
    ASTNode& operator=(ASTNode&&) = delete;

    /// 0-based source offset of the token that produced this node, if the
    /// node came from parsed text. Used to locate evaluation errors.
    std::optional<std::size_t> position() const noexcept { return position_; }

protected:
    explicit ASTNode(std::optional<std::size_t> position) noexcept
        : position_(position)
    {
    }

private:
    std::optional<std::size_t> position_;
};

/// A numeric literal such as 42 or 3.14.
class NumberNode final : public ASTNode {
public:
    explicit NumberNode(double value,
                        std::optional<std::size_t> position = std::nullopt) noexcept;

    double value() const noexcept { return value_; }

private:
    double value_;
};

/// A reference to a named variable such as x.
class VariableNode final : public ASTNode {
public:
    explicit VariableNode(std::string name,
                          std::optional<std::size_t> position = std::nullopt);

    const std::string& name() const noexcept { return name_; }

private:
    std::string name_;
};

/// A prefix operator applied to one operand, e.g. -x.
class UnaryOpNode final : public ASTNode {
public:
    /// Throws std::invalid_argument if operand is null.
    UnaryOpNode(UnaryOperator op,
                ASTNodePtr operand,
                std::optional<std::size_t> position = std::nullopt);

    UnaryOperator op() const noexcept { return op_; }
    const ASTNode& operand() const noexcept { return *operand_; }

private:
    UnaryOperator op_;
    ASTNodePtr operand_;
};

/// An infix operator applied to two operands, e.g. a + b.
class BinaryOpNode final : public ASTNode {
public:
    /// Throws std::invalid_argument if either operand is null.
    BinaryOpNode(BinaryOperator op,
                 ASTNodePtr left,
                 ASTNodePtr right,
                 std::optional<std::size_t> position = std::nullopt);

    BinaryOperator op() const noexcept { return op_; }
    const ASTNode& left() const noexcept { return *left_; }
    const ASTNode& right() const noexcept { return *right_; }

private:
    BinaryOperator op_;
    ASTNodePtr left_;
    ASTNodePtr right_;
};

/// A call such as max(a, b). The parser does not know which functions exist;
/// name and arity are checked when the call is evaluated.
class FunctionCallNode final : public ASTNode {
public:
    /// Throws std::invalid_argument if any argument is null.
    FunctionCallNode(std::string name,
                     std::vector<ASTNodePtr> arguments,
                     std::optional<std::size_t> position = std::nullopt);

    const std::string& name() const noexcept { return name_; }
    std::size_t argumentCount() const noexcept { return arguments_.size(); }

    /// Access by index rather than exposing the vector of unique_ptr: a
    /// `const std::unique_ptr<T>` still hands out a mutable T&, so returning
    /// `const ASTNode&` is what actually keeps the tree read-only.
    /// Precondition: index < argumentCount() (checked; throws std::out_of_range).
    const ASTNode& argument(std::size_t index) const { return *arguments_.at(index); }

private:
    std::string name_;
    std::vector<ASTNodePtr> arguments_;
};

} // namespace expr
