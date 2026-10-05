// Smoke tests: verify that the build, the library link, and the test runner
// are wired together correctly. Real component tests arrive in later phases.

#include "expression/Version.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>
#include <string>

TEST(Smoke, VersionIsNotEmpty)
{
    EXPECT_FALSE(expr::version().empty());
}

TEST(Smoke, VersionHasMajorMinorPatchForm)
{
    const std::string v(expr::version());

    const auto dots = std::count(v.begin(), v.end(), '.');
    EXPECT_EQ(dots, 2) << "version was: " << v;

    const bool onlyDigitsAndDots = std::all_of(v.begin(), v.end(), [](char c) {
        return c == '.' || std::isdigit(static_cast<unsigned char>(c)) != 0;
    });
    EXPECT_TRUE(onlyDigitsAndDots) << "version was: " << v;
}
