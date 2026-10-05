#pragma once

#include "expression/ast/Operators.hpp"
#include "expression/lexer/Token.hpp"

#include <optional>

namespace expr {

enum class Associativity {
    Left,   // a - b - c  ==  (a - b) - c
    Right   // a ^ b ^ c  ==  a ^ (b ^ c)
};

/// Binding strength of operators: a higher level binds more tightly.
///
/// This is the single place where precedence is defined. The parser never
/// compares operators by hand; it only compares these numbers.
///
///   Level  Operators   Associativity  Example
///   -----  ----------  -------------  ----------------------------------
///   1      +  -        left           1 - 2 - 3   ->  (1 - 2) - 3
///   2      *  /  %     left           8 / 4 / 2   ->  (8 / 4) / 2
///   3      unary + -   right          - -3        ->  -(-3)
///   4      ^           right          2 ^ 3 ^ 2   ->  2 ^ (3 ^ 2)
///
/// Unary minus binds *less* tightly than '^', following the mathematical
/// convention (and Python): -2^2 == -(2^2) == -4. It binds *more* tightly
/// than '*', so -x * y == (-x) * y.
namespace precedence {
constexpr int Additive = 1;
constexpr int Multiplicative = 2;
constexpr int Unary = 3;
constexpr int Power = 4;
} // namespace precedence

struct OperatorInfo {
    int precedence;
    Associativity associativity;
};

OperatorInfo operatorInfo(BinaryOperator op) noexcept;
OperatorInfo operatorInfo(UnaryOperator op) noexcept;

/// Token -> operator mapping. Returns nullopt for tokens that cannot act as
/// that kind of operator (e.g. '*' is never a unary operator).
std::optional<BinaryOperator> toBinaryOperator(TokenType type) noexcept;
std::optional<UnaryOperator> toUnaryOperator(TokenType type) noexcept;

} // namespace expr
