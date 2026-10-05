#include "expression/errors/Errors.hpp"

namespace expr {

namespace {

std::string formatMessage(std::string_view category,
                          std::string_view detail,
                          std::optional<std::size_t> position)
{
    std::string message(category);
    message += " error";
    if (position.has_value()) {
        message += " at column ";
        message += std::to_string(*position + 1); // users count from 1
    }
    message += ": ";
    message += detail;
    return message;
}

} // namespace

ExpressionError::ExpressionError(std::string_view category,
                                 std::string_view detail,
                                 std::optional<std::size_t> position)
    : std::runtime_error(formatMessage(category, detail, position))
    , position_(position)
{
}

LexError::LexError(std::string_view detail, std::size_t position)
    : ExpressionError("Lexical", detail, position)
{
}

ParseError::ParseError(std::string_view detail, std::size_t position)
    : ExpressionError("Parse", detail, position)
{
}

} // namespace expr
