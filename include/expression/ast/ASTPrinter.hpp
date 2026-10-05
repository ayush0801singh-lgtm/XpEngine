#pragma once

#include "expression/ast/ASTNode.hpp"

#include <string>

namespace expr {

/// Converts an AST into an S-expression string.
///
/// Example: `3 + 4 * 2` becomes `(+ 3 (* 4 2))`
///
/// This makes precedence and associativity visible and is very useful for
/// testing the parser and debugging.
std::string formatSExpression(const ASTNode& node);

} // namespace expr
