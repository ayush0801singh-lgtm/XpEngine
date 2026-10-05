#pragma once

#include "expression/ast/ASTNode.hpp"
#include "expression/lexer/Token.hpp"

#include <string_view>
#include <vector>

namespace expr {

/// Builds an AST from a token sequence using the Shunting-Yard algorithm.
///
/// `tokens` must be non-empty and end with a TokenType::End token (as
/// produced by expr::tokenize); otherwise std::invalid_argument is thrown.
///
/// Syntax errors throw expr::ParseError with the position of the offending
/// token. The parser checks *syntax only*: whether a function exists or has
/// the right number of arguments, and whether variables are defined, is
/// decided at evaluation time.
ASTNodePtr parse(const std::vector<Token>& tokens);

/// Convenience: tokenize(source) followed by parse(tokens).
/// May throw expr::LexError or expr::ParseError.
ASTNodePtr parse(std::string_view source);

} // namespace expr
