#include "expression/Version.hpp"

#ifndef EXPR_VERSION_STRING
#error "EXPR_VERSION_STRING must be defined by the build system"
#endif

namespace expr {

std::string_view version() noexcept
{
    return EXPR_VERSION_STRING;
}

} // namespace expr
