#include "expression/Engine.hpp"

#include "expression/errors/Errors.hpp"
#include "expression/evaluator/Evaluator.hpp"
#include "expression/lexer/Lexer.hpp"
#include "expression/parser/Parser.hpp"

namespace expr {

Result Engine::evaluate(std::string_view source)
{
    try {
        const auto tokens = tokenize(source);
        const auto ast = parse(tokens);
        return Result(expr::evaluate(*ast, env_));
    } catch (const ExpressionError& e) {
        return Result(Error{e.what(), e.position()});
    } catch (const std::exception& e) {
        return Result(Error{e.what(), std::nullopt});
    }
}

} // namespace expr
