#pragma once

#include <optional>
#include <string>
#include <variant>

namespace expr {

/// Contains information about a failed evaluation.
struct Error {
    std::string message;
    std::optional<std::size_t> position;
};

/// Represents the result of an evaluation: either a numeric value or an error.
/// Designed to emulate std::expected for C++17.
class Result {
public:
    // Constructors
    Result(double value) : data_(value) {}
    Result(Error error) : data_(std::move(error)) {}

    bool hasError() const noexcept {
        return std::holds_alternative<Error>(data_);
    }

    double value() const {
        return std::get<double>(data_);
    }

    const Error& error() const {
        return std::get<Error>(data_);
    }

private:
    std::variant<double, Error> data_;
};

} // namespace expr
