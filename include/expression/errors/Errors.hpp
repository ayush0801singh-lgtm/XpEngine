#pragma once

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace expr {

/// Base class of every error raised by the engine.
///
/// Callers that do not care *which* stage failed can catch ExpressionError;
/// callers that do can catch the concrete subclass (LexError, ...).
///
/// Design note: the only data members are the inherited std::runtime_error
/// message (which standard libraries store in a reference-counted buffer) and
/// a trivially-copyable position. Copying the exception therefore cannot
/// throw, which matters because exceptions are copied during propagation.
class ExpressionError : public std::runtime_error {
public:
    /// 0-based offset into the source text where the problem was detected,
    /// or std::nullopt when no source location is available.
    std::optional<std::size_t> position() const noexcept { return position_; }

protected:
    /// Builds a message of the form
    ///   "<category> error at column <N>: <detail>"
    /// where N is 1-based (editor convention) - or, without a position,
    ///   "<category> error: <detail>".
    ExpressionError(std::string_view category,
                    std::string_view detail,
                    std::optional<std::size_t> position);

private:
    std::optional<std::size_t> position_;
};

/// Raised by the lexer for characters or character sequences that cannot
/// form a valid token (e.g. '$', "1.2.3", "1e+").
class LexError : public ExpressionError {
public:
    LexError(std::string_view detail, std::size_t position);
};

/// Raised by the parser when the token sequence is not a valid expression
/// (missing operands, mismatched parentheses, misplaced commas, ...).
class ParseError : public ExpressionError {
public:
    ParseError(std::string_view detail, std::size_t position);
};

} // namespace expr
