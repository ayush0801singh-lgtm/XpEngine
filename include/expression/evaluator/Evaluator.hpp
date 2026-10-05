#pragma once

#include "expression/ast/ASTNode.hpp"
#include "expression/evaluator/Environment.hpp"

namespace expr {

/// Evaluates an expression tree to a numeric value.
///
/// Uses the provided Environment to resolve variables. Mathematical functions
/// (sin, max, etc.) are built-in.
///
/// Throws expr::EvaluationError for undefined variables, division by zero,
/// or unknown functions.
double evaluate(const ASTNode& root, const Environment& env = Environment());

} // namespace expr
