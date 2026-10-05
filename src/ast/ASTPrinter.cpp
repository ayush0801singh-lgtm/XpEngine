#include "expression/ast/ASTPrinter.hpp"

#include "expression/ast/ASTVisitor.hpp"

#include <sstream>

namespace expr {

namespace {

class SExpressionPrinter final : public ASTVisitor {
public:
    std::string print(const ASTNode& node)
    {
        out_.str("");
        out_.clear();
        node.accept(*this);
        return out_.str();
    }

    void visit(const NumberNode& node) override
    {
        out_ << node.value();
    }

    void visit(const VariableNode& node) override
    {
        out_ << node.name();
    }

    void visit(const UnaryOpNode& node) override
    {
        out_ << '(' << node.op() << ' ';
        node.operand().accept(*this);
        out_ << ')';
    }

    void visit(const BinaryOpNode& node) override
    {
        out_ << '(' << node.op() << ' ';
        node.left().accept(*this);
        out_ << ' ';
        node.right().accept(*this);
        out_ << ')';
    }

    void visit(const FunctionCallNode& node) override
    {
        out_ << '(' << node.name();
        for (std::size_t i = 0; i < node.argumentCount(); ++i) {
            out_ << ' ';
            node.argument(i).accept(*this);
        }
        out_ << ')';
    }

private:
    std::ostringstream out_;
};

} // namespace

std::string formatSExpression(const ASTNode& node)
{
    return SExpressionPrinter().print(node);
}

} // namespace expr
