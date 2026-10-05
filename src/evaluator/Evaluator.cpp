#include "expression/evaluator/Evaluator.hpp"

#include "expression/ast/ASTVisitor.hpp"
#include "expression/evaluator/EvaluationError.hpp"

#include <cmath>
#include <vector>

namespace expr {

namespace {

class EvaluatorVisitor final : public ASTVisitor {
public:
    explicit EvaluatorVisitor(const Environment& env) noexcept
        : env_(env)
    {
    }

    double result() const noexcept { return result_; }

    void visit(const NumberNode& node) override
    {
        result_ = node.value();
    }

    void visit(const VariableNode& node) override
    {
        result_ = env_.get(node.name(), node.position());
    }

    void visit(const UnaryOpNode& node) override
    {
        node.operand().accept(*this);
        const double operand = result_;

        switch (node.op()) {
        case UnaryOperator::Plus:
            result_ = operand;
            break;
        case UnaryOperator::Minus:
            result_ = -operand;
            break;
        }
    }

    void visit(const BinaryOpNode& node) override
    {
        node.left().accept(*this);
        const double left = result_;

        node.right().accept(*this);
        const double right = result_;

        switch (node.op()) {
        case BinaryOperator::Add:
            result_ = left + right;
            break;
        case BinaryOperator::Subtract:
            result_ = left - right;
            break;
        case BinaryOperator::Multiply:
            result_ = left * right;
            break;
        case BinaryOperator::Divide:
            if (right == 0.0) {
                throw EvaluationError("division by zero", node.position());
            }
            result_ = left / right;
            break;
        case BinaryOperator::Modulo:
            if (right == 0.0) {
                throw EvaluationError("modulo by zero", node.position());
            }
            result_ = std::fmod(left, right);
            break;
        case BinaryOperator::Power:
            result_ = std::pow(left, right);
            break;
        }
    }

    void visit(const FunctionCallNode& node) override
    {
        std::vector<double> args;
        args.reserve(node.argumentCount());
        for (std::size_t i = 0; i < node.argumentCount(); ++i) {
            node.argument(i).accept(*this);
            args.push_back(result_);
        }

        result_ = evaluateFunction(node.name(), args, node.position());
    }

private:
    [[noreturn]] void throwArityError(std::string_view name,
                                      std::size_t expected,
                                      std::size_t got,
                                      std::optional<std::size_t> position) const
    {
        throw EvaluationError("function '" + std::string(name) + "' expects " +
                                  std::to_string(expected) + " argument(s), got " +
                                  std::to_string(got),
                              position);
    }

    void requireArguments(std::string_view name,
                          std::size_t expected,
                          std::size_t got,
                          std::optional<std::size_t> position) const
    {
        if (got != expected) {
            throwArityError(name, expected, got, position);
        }
    }

    double evaluateFunction(std::string_view name,
                            const std::vector<double>& args,
                            std::optional<std::size_t> position) const
    {
        if (name == "sin") {
            requireArguments(name, 1, args.size(), position);
            return std::sin(args[0]);
        }
        if (name == "cos") {
            requireArguments(name, 1, args.size(), position);
            return std::cos(args[0]);
        }
        if (name == "tan") {
            requireArguments(name, 1, args.size(), position);
            return std::tan(args[0]);
        }
        if (name == "sqrt") {
            requireArguments(name, 1, args.size(), position);
            if (args[0] < 0.0) {
                throw EvaluationError("sqrt of negative number", position);
            }
            return std::sqrt(args[0]);
        }
        if (name == "log") {
            requireArguments(name, 1, args.size(), position);
            if (args[0] <= 0.0) {
                throw EvaluationError("log of non-positive number", position);
            }
            return std::log(args[0]);
        }
        if (name == "exp") {
            requireArguments(name, 1, args.size(), position);
            return std::exp(args[0]);
        }
        if (name == "abs") {
            requireArguments(name, 1, args.size(), position);
            return std::abs(args[0]);
        }
        if (name == "min") {
            requireArguments(name, 2, args.size(), position);
            return std::min(args[0], args[1]);
        }
        if (name == "max") {
            requireArguments(name, 2, args.size(), position);
            return std::max(args[0], args[1]);
        }

        throw EvaluationError("unknown function '" + std::string(name) + "'", position);
    }

    const Environment& env_;
    double result_ = 0.0;
};

} // namespace

double evaluate(const ASTNode& root, const Environment& env)
{
    EvaluatorVisitor visitor(env);
    root.accept(visitor);
    return visitor.result();
}

} // namespace expr
