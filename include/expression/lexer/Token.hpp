#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>
#include <string_view>

namespace expr {

/// Every kind of token the lexer can produce.
///
/// Note there is deliberately no "UnaryMinus" token: whether '-' is unary or
/// binary depends on grammatical context (what came before it), which is the
/// parser's job. The lexer only reports *what characters* it saw.
enum class TokenType {
    Number,      // 42, 3.14, .5, 1e-3
    Identifier,  // x, sin, _tmp, var2
    Plus,        // +
    Minus,       // -
    Star,        // *
    Slash,       // /
    Percent,     // %
    Caret,       // ^
    LeftParen,   // (
    RightParen,  // )
    Comma,       // ,
    End          // end of input (always the last token)
};

/// Human-readable description used in error messages and test output,
/// e.g. "number", "'+'", "end of input".
std::string_view toString(TokenType type) noexcept;

/// Lets GoogleTest (and any std::ostream) print token types readably.
std::ostream& operator<<(std::ostream& os, TokenType type);

/// A single lexical unit.
///
/// The token owns a copy of its source text (lexeme), so a token sequence
/// stays valid after the original input string is destroyed.
struct Token {
    TokenType type = TokenType::End;
    std::string lexeme;        // exact text from the source ("" for End)
    double number = 0.0;       // numeric value; meaningful only for Number
    std::size_t position = 0;  // 0-based offset of the first character
};

} // namespace expr
