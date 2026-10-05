#include "expression/evaluator/Environment.hpp"

#include <utility>

namespace expr {

void Environment::set(std::string name, double value)
{
    variables_.insert_or_assign(std::move(name), value);
}

double Environment::get(std::string_view name,
                        std::optional<std::size_t> position) const
{
    const auto it = variables_.find(name);
    if (it == variables_.end()) {
        throw EvaluationError("undefined variable '" + std::string(name) + "'",
                              position);
    }
    return it->second;
}

} // namespace expr
