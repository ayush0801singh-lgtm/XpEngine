# ---------------------------------------------------------------------------
# Test framework: GoogleTest.
# Prefer an installed GoogleTest (offline builds, distro packages); otherwise
# download a pinned, hash-verified release.
# ---------------------------------------------------------------------------
find_package(GTest 1.14 CONFIG QUIET)

if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(googletest
        URL      https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
        URL_HASH SHA256=40d4ec942217dcc84a9ebe2a68584ada7d4a33a8ee958755763278ea1c5e18ff
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    # MSVC: use the same (dynamic) C runtime as the rest of the project.
    set(gtest_force_shared_crt ON  CACHE BOOL "" FORCE)
    set(BUILD_GMOCK            OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST          OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(expression_tests
    test_smoke.cpp
    test_lexer.cpp
)

target_link_libraries(expression_tests PRIVATE expr::core GTest::gtest_main)
set_target_properties(expression_tests PROPERTIES CXX_EXTENSIONS OFF)

expr_enable_warnings(expression_tests)
expr_configure_executable(expression_tests)

# Register every TEST() with CTest individually so `ctest` reports per-test.
include(GoogleTest)
gtest_discover_tests(expression_tests DISCOVERY_MODE PRE_TEST)
