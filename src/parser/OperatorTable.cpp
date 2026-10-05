#include "expression/parser/OperatorTable.hpp"

namespace expr {

OperatorInfo operatorInfo(BinaryOperator op) noexcept
{
    switch (op) {
    case BinaryOperator::Add:
    case BinaryOperator::Subtract:
        return {precedence::Additive, Associativity::Left};
    case BinaryOperator::Multiply:
    case BinaryOperator::Divide:
    case BinaryOperator::Modulo:
        return {precedence::Multiplicative, Associativity::Left};
    case BinaryOperator::Power:
        return {precedence::Power, Associativity::Right};
    }
    return {precedence::Additive, Associativity::Left}; // unreachable
}

OperatorInfo operatorInfo(UnaryOperator op) noexcept
{
    switch (op) {
    case UnaryOperator::Plus:
    case UnaryOperator::Minus:
        return {precedence::Unary, Associativity::Right};
    }
    return {precedence::Unary, Associativity::Right}; // unreachable
}

std::optional<BinaryOperator> toBinaryOperator(TokenType type) noexcept
{
    switch (type) {
    case TokenType::Plus:    return BinaryOperator::Add;
    case TokenType::Minus:   return BinaryOperator::Subtract;
    case TokenType::Star:    return BinaryOperator::Multiply;
    case TokenType::Slash:   return BinaryOperator::Divide;
    case TokenType::Percent: return BinaryOperator::Modulo;
    case TokenType::Caret:   return BinaryOperator::Power;
    default:                 return std::nullopt;
    }
}

std::optional<UnaryOperator> toUnaryOperator(TokenType type) noexcept
{
    switch (type) {
    case TokenType::Plus:  return UnaryOperator::Plus;
    case TokenType::Minus: return UnaryOperator::Minus;
    default:               return std::nullopt;
    }
}

} // namespace expr
