#pragma once

#include "expression/evaluator/EvaluationError.hpp"

#include <string>
#include <string_view>
#include <unordered_map>

namespace expr {

/// Stores the values of variables used in expressions.
class Environment {
public:
    /// Assigns or updates a variable.
    void set(std::string name, double value);

    /// Looks up a variable. Throws EvaluationError if not found.
    double get(std::string_view name,
               std::optional<std::size_t> position = std::nullopt) const;

private:
    // Transparent comparator allows lookup with string_view without allocating
    // a temporary std::string just to query the map.
    struct StringHash {
        using is_transparent = void;
        std::size_t operator()(std::string_view sv) const {
            return std::hash<std::string_view>{}(sv);
        }
    };

    std::unordered_map<std::string, double, StringHash, std::equal_to<>> variables_;
};

} // namespace expr
