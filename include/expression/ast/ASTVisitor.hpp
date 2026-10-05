#pragma once

namespace expr {

class NumberNode;
class VariableNode;
class UnaryOpNode;
class BinaryOpNode;
class FunctionCallNode;

/// Interface for operations that process an AST without modifying it.
///
/// The AST nodes are unaware of evaluation, printing, or optimisation.
/// Adding a new operation just requires writing a new Visitor.
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(const NumberNode& node) = 0;
    virtual void visit(const VariableNode& node) = 0;
    virtual void visit(const UnaryOpNode& node) = 0;
    virtual void visit(const BinaryOpNode& node) = 0;
    virtual void visit(const FunctionCallNode& node) = 0;
};

} // namespace expr
