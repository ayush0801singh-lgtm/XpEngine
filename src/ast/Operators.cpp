#include "expression/ast/Operators.hpp"

#include <ostream>

namespace expr {

std::string_view symbol(UnaryOperator op) noexcept
{
    switch (op) {
    case UnaryOperator::Plus:  return "+";
    case UnaryOperator::Minus: return "-";
    }
    return "?";
}

std::string_view symbol(BinaryOperator op) noexcept
{
    switch (op) {
    case BinaryOperator::Add:      return "+";
    case BinaryOperator::Subtract: return "-";
    case BinaryOperator::Multiply: return "*";
    case BinaryOperator::Divide:   return "/";
    case BinaryOperator::Modulo:   return "%";
    case BinaryOperator::Power:    return "^";
    }
    return "?";
}

std::ostream& operator<<(std::ostream& os, UnaryOperator op)
{
    return os << symbol(op);
}

std::ostream& operator<<(std::ostream& os, BinaryOperator op)
{
    return os << symbol(op);
}

} // namespace expr
