#include "expression/lexer/Lexer.hpp"

#include "expression/errors/Errors.hpp"

#include <charconv>
#include <optional>
#include <string>
#include <system_error>
#include <utility>

namespace expr {

namespace {

// Character classification is done with explicit ASCII ranges rather than
// <cctype> (std::isdigit etc.). The <cctype> functions depend on the current
// C locale and have undefined behaviour for negative char values, which occur
// for non-ASCII bytes on platforms where char is signed.

constexpr bool isDigit(char c) noexcept
{
    return c >= '0' && c <= '9';
}

constexpr bool isIdentifierStart(char c) noexcept
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

constexpr bool isIdentifierChar(char c) noexcept
{
    return isIdentifierStart(c) || isDigit(c);
}

constexpr bool isWhitespace(char c) noexcept
{
    switch (c) {
    case ' ': case '\t': case '\n': case '\r': case '\f': case '\v':
        return true;
    default:
        return false;
    }
}

/// Maps characters that form a complete token on their own.
/// Keeping this in one table means adding an operator is a one-line change.
std::optional<TokenType> singleCharTokenType(char c) noexcept
{
    switch (c) {
    case '+': return TokenType::Plus;
    case '-': return TokenType::Minus;
    case '*': return TokenType::Star;
    case '/': return TokenType::Slash;
    case '%': return TokenType::Percent;
    case '^': return TokenType::Caret;
    case '(': return TokenType::LeftParen;
    case ')': return TokenType::RightParen;
    case ',': return TokenType::Comma;
    default:  return std::nullopt;
    }
}

/// Renders a character for an error message: printable ASCII as 'c',
/// anything else (control characters, UTF-8 bytes) as a hex byte value.
std::string describeChar(char c)
{
    const auto byte = static_cast<unsigned char>(c);
    if (byte >= 0x20 && byte < 0x7F) {
        return std::string("'") + c + "'";
    }
    constexpr char hexDigits[] = "0123456789ABCDEF";
    std::string text = "byte 0x";
    text += hexDigits[byte >> 4U];
    text += hexDigits[byte & 0x0FU];
    return text;
}

/// Converts already-validated number text to a double.
///
/// std::from_chars is used instead of std::stod/strtod because it is
/// locale-independent (strtod would expect ',' as decimal separator under
/// e.g. a German locale), does not allocate, and reports range errors
/// without relying on errno.
double toDouble(std::string_view text, std::size_t position)
{
    double value = 0.0;
    const char* const first = text.data();
    const char* const last = text.data() + text.size();
    const auto [ptr, ec] = std::from_chars(first, last, value);

    if (ec == std::errc::result_out_of_range) {
        throw LexError("number '" + std::string(text) +
                           "' is out of range for a double",
                       position);
    }
    if (ec != std::errc() || ptr != last) {
        // The lexer validates the grammar before calling this, so reaching
        // here indicates a bug in the lexer rather than bad user input.
        throw LexError("internal error: could not convert number '" +
                           std::string(text) + "'",
                       position);
    }
    return value;
}

} // namespace

Lexer::Lexer(std::string_view source) noexcept
    : source_(source)
{
}

bool Lexer::atEnd() const noexcept
{
    return pos_ >= source_.size();
}

char Lexer::peek(std::size_t offset) const noexcept
{
    const std::size_t index = pos_ + offset;
    return index < source_.size() ? source_[index] : '\0';
}

void Lexer::skipWhitespace() noexcept
{
    while (!atEnd() && isWhitespace(peek())) {
        ++pos_;
    }
}

void Lexer::consumeDigits() noexcept
{
    while (isDigit(peek())) {
        ++pos_;
    }
}

Token Lexer::next()
{
    skipWhitespace();

    if (atEnd()) {
        return Token{TokenType::End, {}, 0.0, pos_};
    }

    const char c = peek();

    // A number starts with a digit, or with '.' immediately followed by a
    // digit (".5"). A lone '.' falls through and is reported as invalid.
    if (isDigit(c) || (c == '.' && isDigit(peek(1)))) {
        return lexNumber();
    }

    if (isIdentifierStart(c)) {
        return lexIdentifier();
    }

    if (const auto type = singleCharTokenType(c)) {
        const std::size_t start = pos_;
        ++pos_;
        return Token{*type, std::string(1, c), 0.0, start};
    }

    throw LexError("unexpected character " + describeChar(c), pos_);
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;
    while (true) {
        Token token = next();
        const bool isEnd = (token.type == TokenType::End);
        tokens.push_back(std::move(token));
        if (isEnd) {
            break;
        }
    }
    return tokens;
}

// Grammar accepted for numbers (no sign: '-' is always a separate token):
//
//   number   := digits [ '.' digits ] [ exponent ]
//             | '.' digits [ exponent ]
//   exponent := ( 'e' | 'E' ) [ '+' | '-' ] digits
//
// A number must not be immediately followed by '.', a digit or a letter;
// "1.2.3", "12abc" and "1e5x" are reported as malformed numbers rather than
// silently split into several tokens.
Token Lexer::lexNumber()
{
    const std::size_t start = pos_;

    consumeDigits(); // integer part (empty for ".5")

    if (peek() == '.') {
        ++pos_;
        if (!isDigit(peek())) {
            throwMalformedNumber(start, "expected digit after decimal point");
        }
        consumeDigits();
    }

    if (peek() == 'e' || peek() == 'E') {
        ++pos_;
        if (peek() == '+' || peek() == '-') {
            ++pos_;
        }
        if (!isDigit(peek())) {
            throwMalformedNumber(start, "exponent has no digits");
        }
        consumeDigits();
    }

    if (peek() == '.') {
        throwMalformedNumber(start, "unexpected '.' in number");
    }
    if (isIdentifierChar(peek())) {
        throwMalformedNumber(start,
                             "unexpected character " + describeChar(peek()) +
                                 " in number");
    }

    const std::string_view text = source_.substr(start, pos_ - start);
    return Token{TokenType::Number, std::string(text), toDouble(text, start), start};
}

Token Lexer::lexIdentifier()
{
    const std::size_t start = pos_;
    while (isIdentifierChar(peek())) {
        ++pos_;
    }
    return Token{TokenType::Identifier,
                 std::string(source_.substr(start, pos_ - start)),
                 0.0,
                 start};
}

void Lexer::throwMalformedNumber(std::size_t start, std::string_view reason) const
{
    // Show the user the whole malformed run (e.g. "1.2.3", not just "1.2"),
    // while pointing the position at the exact offending character.
    std::size_t end = pos_;
    while (end < source_.size() &&
           (isIdentifierChar(source_[end]) || source_[end] == '.')) {
        ++end;
    }
    const std::string_view text = source_.substr(start, end - start);
    throw LexError("malformed number '" + std::string(text) + "': " +
                       std::string(reason),
                   pos_);
}

std::vector<Token> tokenize(std::string_view source)
{
    return Lexer(source).tokenize();
}

} // namespace expr
