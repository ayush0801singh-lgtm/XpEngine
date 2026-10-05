#pragma once

#include "expression/lexer/Token.hpp"

#include <cstddef>
#include <string_view>
#include <vector>

namespace expr {

/// Converts source text into a sequence of tokens.
///
/// The lexer is a hand-written single-pass scanner: each call to next()
/// looks at the current character, decides which kind of token starts there,
/// and consumes exactly the characters belonging to that token.
///
/// Lifetime: the lexer stores a non-owning view of the source, so the source
/// string must outlive the Lexer object. The Tokens it returns own their text
/// and have no such restriction.
///
/// Errors: throws expr::LexError for invalid characters and malformed numbers.
class Lexer {
public:
    explicit Lexer(std::string_view source) noexcept;

    /// Returns the next token. Once the input is exhausted it returns an End
    /// token, and keeps returning End on every subsequent call.
    Token next();

    /// Lexes the remaining input. The returned vector always ends with
    /// exactly one End token.
    std::vector<Token> tokenize();

private:
    bool atEnd() const noexcept;
    char peek(std::size_t offset = 0) const noexcept;

    void skipWhitespace() noexcept;
    void consumeDigits() noexcept;

    Token lexNumber();
    Token lexIdentifier();

    [[noreturn]] void throwMalformedNumber(std::size_t start,
                                           std::string_view reason) const;

    std::string_view source_;
    std::size_t pos_ = 0;
};

/// Convenience wrapper: Lexer(source).tokenize().
std::vector<Token> tokenize(std::string_view source);

} // namespace expr
