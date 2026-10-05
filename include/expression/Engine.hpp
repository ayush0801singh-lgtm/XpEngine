#pragma once

#include "expression/Result.hpp"
#include "expression/evaluator/Environment.hpp"

#include <string_view>

namespace expr {

/// The main entry point for the expression engine.
/// Provides a safe, non-throwing API for evaluating expressions.
class Engine {
public:
    /// Evaluates an expression string using the current environment.
    /// Does not throw on syntax or evaluation errors; returns a Result instead.
    Result evaluate(std::string_view source);

    /// Provides access to the variable environment.
    Environment& environment() noexcept { return env_; }
    const Environment& environment() const noexcept { return env_; }

private:
    Environment env_;
};

} // namespace expr
