#include "expression/lexer/Token.hpp"

#include <ostream>

namespace expr {

std::string_view toString(TokenType type) noexcept
{
    switch (type) {
    case TokenType::Number:     return "number";
    case TokenType::Identifier: return "identifier";
    case TokenType::Plus:       return "'+'";
    case TokenType::Minus:      return "'-'";
    case TokenType::Star:       return "'*'";
    case TokenType::Slash:      return "'/'";
    case TokenType::Percent:    return "'%'";
    case TokenType::Caret:      return "'^'";
    case TokenType::LeftParen:  return "'('";
    case TokenType::RightParen: return "')'";
    case TokenType::Comma:      return "','";
    case TokenType::End:        return "end of input";
    }
    return "unknown token"; // unreachable for valid enum values
}

std::ostream& operator<<(std::ostream& os, TokenType type)
{
    return os << toString(type);
}

} // namespace expr
