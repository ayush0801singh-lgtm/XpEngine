#pragma once

#include <string_view>

namespace expr {

/// Returns the library version in "MAJOR.MINOR.PATCH" form.
/// The value comes from the project() version in CMakeLists.txt.
std::string_view version() noexcept;

} // namespace expr
