#pragma once

#include "expression/errors/Errors.hpp"

namespace expr {

/// Raised by the evaluator for mathematical or runtime errors
/// (e.g. undefined variable, division by zero, unknown function).
class EvaluationError : public ExpressionError {
public:
    EvaluationError(std::string_view detail,
                    std::optional<std::size_t> position = std::nullopt);
};

} // namespace expr
