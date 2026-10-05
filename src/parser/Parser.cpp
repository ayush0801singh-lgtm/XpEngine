#include "expression/parser/Parser.hpp"

#include "expression/errors/Errors.hpp"
#include "expression/lexer/Lexer.hpp"
#include "expression/parser/OperatorTable.hpp"

#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

// ===========================================================================
// Shunting-Yard parser that produces an AST
// ===========================================================================
//
// Two stacks are maintained:
//
//   operands_   - finished subtrees (ASTNodePtr)
//   operators_  - operators and open parentheses still waiting for their
//                 right-hand side to be complete
//
// Instead of emitting Reverse Polish Notation, "reducing" an operator pops
// its operands off operands_, builds a node, and pushes the node back. When
// the input is exhausted, exactly one operand - the root - remains.
//
// The parser is a two-state machine. `expectOperand_` says what may legally
// come next:
//
//   expecting an OPERAND  : number, identifier, function call, '(' or a
//                           prefix operator (+ / -). Here '+'/'-' are UNARY.
//   expecting an OPERATOR : binary operator, ')', ',' or end of input.
//                           Here '+'/'-' are BINARY.
//
// This state is what distinguishes unary from binary minus, and it is also
// what detects most syntax errors: any token that is illegal in the current
// state is reported immediately, with that token's position. As a result
// the operand stack can never underflow for valid *or* invalid input.
// ===========================================================================

namespace expr {

namespace {

enum class PendingKind {
    Unary,   // prefix operator awaiting its operand
    Binary,  // infix operator awaiting its right operand
    Group,   // '(' used for grouping
    Call     // 'name(' of a function call
};

/// An entry on the operator stack. Only the fields relevant to `kind` are
/// meaningful; a tagged struct is used rather than std::variant to keep the
/// algorithm easy to read.
struct PendingOperator {
    PendingKind kind = PendingKind::Group;
    std::size_t position = 0;                       // token position
    UnaryOperator unaryOp = UnaryOperator::Plus;    // kind == Unary
    BinaryOperator binaryOp = BinaryOperator::Add;  // kind == Binary
    std::string functionName;                       // kind == Call
    std::size_t commaCount = 0;                     // kind == Call

    static PendingOperator unary(UnaryOperator op, std::size_t position)
    {
        PendingOperator entry;
        entry.kind = PendingKind::Unary;
        entry.position = position;
        entry.unaryOp = op;
        return entry;
    }

    static PendingOperator binary(BinaryOperator op, std::size_t position)
    {
        PendingOperator entry;
        entry.kind = PendingKind::Binary;
        entry.position = position;
        entry.binaryOp = op;
        return entry;
    }

    static PendingOperator group(std::size_t position)
    {
        PendingOperator entry;
        entry.kind = PendingKind::Group;
        entry.position = position;
        return entry;
    }

    static PendingOperator call(std::string name, std::size_t position)
    {
        PendingOperator entry;
        entry.kind = PendingKind::Call;
        entry.position = position;
        entry.functionName = std::move(name);
        return entry;
    }

    bool isOperator() const noexcept
    {
        return kind == PendingKind::Unary || kind == PendingKind::Binary;
    }

    int precedence() const noexcept
    {
        return kind == PendingKind::Unary ? operatorInfo(unaryOp).precedence
                                          : operatorInfo(binaryOp).precedence;
    }
};

/// Describes a token for error messages: "number '4'", "identifier 'x'", "'+'".
std::string describe(const Token& token)
{
    switch (token.type) {
    case TokenType::Number:
        return "number '" + token.lexeme + "'";
    case TokenType::Identifier:
        return "identifier '" + token.lexeme + "'";
    default:
        return std::string(toString(token.type));
    }
}

class ShuntingYardParser {
public:
    explicit ShuntingYardParser(const std::vector<Token>& tokens) noexcept
        : tokens_(tokens)
    {
    }

    ASTNodePtr run();

private:
    void handleOperandPosition(const Token& token);
    void handleOperatorPosition(const Token& token);

    void pushBinaryOperator(BinaryOperator op, std::size_t position);
    void closeParenthesis(const Token& token);
    void handleComma(const Token& token);
    ASTNodePtr finish();

    void reduceUntilOpenParen();
    void reduceTop();
    void completeCall(const PendingOperator& call, std::size_t argumentCount);
    ASTNodePtr popOperand();

    bool isEmptyCallClose() const noexcept;
    TokenType peekType(std::size_t offset) const noexcept;
    [[noreturn]] void throwMissingOperand(const Token& token) const;

    const std::vector<Token>& tokens_;
    std::size_t index_ = 0;
    bool expectOperand_ = true;
    std::vector<ASTNodePtr> operands_;
    std::vector<PendingOperator> operators_;
};

ASTNodePtr ShuntingYardParser::run()
{
    for (index_ = 0; index_ < tokens_.size(); ++index_) {
        const Token& token = tokens_[index_];

        if (expectOperand_) {
            handleOperandPosition(token); // throws on End: an operand is missing
        } else if (token.type == TokenType::End) {
            break;
        } else {
            handleOperatorPosition(token);
        }
    }
    return finish();
}

void ShuntingYardParser::handleOperandPosition(const Token& token)
{
    switch (token.type) {
    case TokenType::Number:
        operands_.push_back(std::make_unique<NumberNode>(token.number, token.position));
        expectOperand_ = false;
        return;

    case TokenType::Identifier:
        if (peekType(1) == TokenType::LeftParen) {
            // Function call: the '(' is consumed together with the name, and
            // we stay in operand state for the first argument.
            operators_.push_back(PendingOperator::call(token.lexeme, token.position));
            ++index_;
            return;
        }
        operands_.push_back(std::make_unique<VariableNode>(token.lexeme, token.position));
        expectOperand_ = false;
        return;

    case TokenType::LeftParen:
        operators_.push_back(PendingOperator::group(token.position));
        return;

    case TokenType::RightParen:
        if (isEmptyCallClose()) { // "f()" - a call with zero arguments
            const PendingOperator call = std::move(operators_.back());
            operators_.pop_back();
            completeCall(call, 0);
            expectOperand_ = false;
            return;
        }
        break;

    default:
        // In operand position '+' and '-' are prefix (unary) operators.
        // A prefix operator has no left operand, so it never forces earlier
        // operators to be reduced - it is simply pushed.
        if (const auto op = toUnaryOperator(token.type)) {
            operators_.push_back(PendingOperator::unary(*op, token.position));
            return;
        }
        break;
    }

    throwMissingOperand(token);
}

void ShuntingYardParser::handleOperatorPosition(const Token& token)
{
    if (const auto op = toBinaryOperator(token.type)) {
        pushBinaryOperator(*op, token.position);
        expectOperand_ = true;
        return;
    }

    switch (token.type) {
    case TokenType::RightParen:
        closeParenthesis(token);
        return; // a closed group/call is itself an operand: still expect an operator

    case TokenType::Comma:
        handleComma(token);
        expectOperand_ = true;
        return;

    default:
        // A number, identifier or '(' directly after a complete operand,
        // e.g. "2 3", "2 x" or "2 (3)". Implicit multiplication is not
        // supported, so this is an error.
        throw ParseError("unexpected " + describe(token) +
                             " after a complete expression (missing operator?)",
                         token.position);
    }
}

void ShuntingYardParser::pushBinaryOperator(BinaryOperator op, std::size_t position)
{
    const OperatorInfo incoming = operatorInfo(op);

    // Reduce every pending operator that binds at least as tightly as the
    // incoming one. For equal precedence, a left-associative incoming
    // operator reduces the pending one first ((a - b) - c); a
    // right-associative one waits (a ^ (b ^ c)).
    while (!operators_.empty() && operators_.back().isOperator()) {
        const int pending = operators_.back().precedence();
        const bool reducePending =
            pending > incoming.precedence ||
            (pending == incoming.precedence &&
             incoming.associativity == Associativity::Left);
        if (!reducePending) {
            break;
        }
        reduceTop();
    }

    operators_.push_back(PendingOperator::binary(op, position));
}

void ShuntingYardParser::closeParenthesis(const Token& token)
{
    reduceUntilOpenParen();

    if (operators_.empty()) {
        throw ParseError("unmatched ')'", token.position);
    }

    const PendingOperator open = std::move(operators_.back());
    operators_.pop_back();

    if (open.kind == PendingKind::Call) {
        completeCall(open, open.commaCount + 1);
    }
    // For a grouping '(' nothing else is needed: the enclosed expression has
    // already been reduced to a single operand. Parentheses leave no trace
    // in the AST - the tree shape itself encodes the grouping.
}

void ShuntingYardParser::handleComma(const Token& token)
{
    reduceUntilOpenParen(); // finish the argument that precedes the comma

    if (operators_.empty() || operators_.back().kind != PendingKind::Call) {
        throw ParseError("',' is only allowed between function arguments",
                         token.position);
    }
    ++operators_.back().commaCount;
}

ASTNodePtr ShuntingYardParser::finish()
{
    while (!operators_.empty()) {
        const PendingOperator& top = operators_.back();
        if (top.kind == PendingKind::Group) {
            throw ParseError("unmatched '('", top.position);
        }
        if (top.kind == PendingKind::Call) {
            throw ParseError("missing ')' to close call to '" + top.functionName + "'",
                             top.position);
        }
        reduceTop();
    }

    if (operands_.size() != 1) {
        throw std::logic_error("parser internal error: expected exactly one result");
    }
    return popOperand();
}

void ShuntingYardParser::reduceUntilOpenParen()
{
    while (!operators_.empty() && operators_.back().isOperator()) {
        reduceTop();
    }
}

void ShuntingYardParser::reduceTop()
{
    const PendingOperator entry = std::move(operators_.back());
    operators_.pop_back();

    if (entry.kind == PendingKind::Unary) {
        ASTNodePtr operand = popOperand();
        operands_.push_back(
            std::make_unique<UnaryOpNode>(entry.unaryOp, std::move(operand), entry.position));
    } else if (entry.kind == PendingKind::Binary) {
        // Right operand is on top of the stack, left operand below it.
        ASTNodePtr right = popOperand();
        ASTNodePtr left = popOperand();
        operands_.push_back(std::make_unique<BinaryOpNode>(
            entry.binaryOp, std::move(left), std::move(right), entry.position));
    } else {
        throw std::logic_error("parser internal error: cannot reduce a parenthesis");
    }
}

void ShuntingYardParser::completeCall(const PendingOperator& call, std::size_t argumentCount)
{
    if (operands_.size() < argumentCount) {
        throw std::logic_error("parser internal error: missing function arguments");
    }

    // The arguments are the top `argumentCount` operands, in source order.
    const auto first = operands_.end() - static_cast<std::ptrdiff_t>(argumentCount);
    std::vector<ASTNodePtr> arguments(std::make_move_iterator(first),
                                      std::make_move_iterator(operands_.end()));
    operands_.erase(first, operands_.end());

    operands_.push_back(std::make_unique<FunctionCallNode>(
        call.functionName, std::move(arguments), call.position));
}

ASTNodePtr ShuntingYardParser::popOperand()
{
    // Unreachable for any input thanks to the operand/operator state machine;
    // kept as a defensive check against future changes to the algorithm.
    if (operands_.empty()) {
        throw std::logic_error("parser internal error: operand stack underflow");
    }
    ASTNodePtr node = std::move(operands_.back());
    operands_.pop_back();
    return node;
}

bool ShuntingYardParser::isEmptyCallClose() const noexcept
{
    // ')' immediately after 'name(' : the '(' token was consumed together
    // with the function name, so the call entry is on top of the stack.
    return index_ > 0 &&
           tokens_[index_ - 1].type == TokenType::LeftParen &&
           !operators_.empty() &&
           operators_.back().kind == PendingKind::Call;
}

TokenType ShuntingYardParser::peekType(std::size_t offset) const noexcept
{
    const std::size_t index = index_ + offset;
    return index < tokens_.size() ? tokens_[index].type : TokenType::End;
}

void ShuntingYardParser::throwMissingOperand(const Token& token) const
{
    if (index_ == 0 && token.type == TokenType::End) {
        throw ParseError("empty expression", token.position);
    }

    std::string message = "expected expression";
    if (index_ > 0) {
        message += " after " + describe(tokens_[index_ - 1]);
    }
    message += ", found " + describe(token);
    throw ParseError(message, token.position);
}

} // namespace

ASTNodePtr parse(const std::vector<Token>& tokens)
{
    if (tokens.empty() || tokens.back().type != TokenType::End) {
        throw std::invalid_argument("parse: token sequence must end with an End token");
    }
    return ShuntingYardParser(tokens).run();
}

ASTNodePtr parse(std::string_view source)
{
    const std::vector<Token> tokens = tokenize(source);
    return parse(tokens);
}

} // namespace expr
