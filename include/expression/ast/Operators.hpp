#pragma once

#include <iosfwd>
#include <string_view>

namespace expr {

/// Prefix operators that can appear in an expression.
enum class UnaryOperator {
    Plus,   // +x
    Minus   // -x
};

/// Infix operators that can appear in an expression.
enum class BinaryOperator {
    Add,       // a + b
    Subtract,  // a - b
    Multiply,  // a * b
    Divide,    // a / b
    Modulo,    // a % b
    Power      // a ^ b
};

/// Source symbol of the operator, e.g. "-" or "^".
std::string_view symbol(UnaryOperator op) noexcept;
std::string_view symbol(BinaryOperator op) noexcept;

std::ostream& operator<<(std::ostream& os, UnaryOperator op);
std::ostream& operator<<(std::ostream& os, BinaryOperator op);

} // namespace expr
