cmake_minimum_required(VERSION 3.16)

project(ExpressionEngine
    VERSION 0.1.0
    DESCRIPTION "Expression parser and AST evaluation engine"
    LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Guard against accidental in-source builds (they pollute the source tree).
# ---------------------------------------------------------------------------
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are not allowed. Use: cmake -S . -B build")
endif()

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(EXPR_BUILD_APP           "Build the command-line application" ON)
option(EXPR_BUILD_TESTS         "Build the unit test suite"          ON)
option(EXPR_WARNINGS_AS_ERRORS  "Treat compiler warnings as errors"  OFF)

# Default to a Release build for single-config generators (Ninja, Makefiles)
# when the user did not specify one.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(ProjectSettings)

# ---------------------------------------------------------------------------
# Core library: lexer, parser, AST, evaluator, errors.
# The CLI and the tests both link against this; neither contains core logic.
# ---------------------------------------------------------------------------
add_library(expression_core STATIC
    src/Version.cpp
    src/errors/Errors.cpp
    src/lexer/Token.cpp
    src/lexer/Lexer.cpp
    src/ast/Operators.cpp
    src/ast/ASTNode.cpp
    src/ast/ASTPrinter.cpp
    src/parser/OperatorTable.cpp
    src/parser/Parser.cpp
    src/evaluator/Environment.cpp
    src/evaluator/Evaluator.cpp
    src/Engine.cpp
)
add_library(expr::core ALIAS expression_core)

target_include_directories(expression_core
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

target_compile_features(expression_core PUBLIC cxx_std_17)
set_target_properties(expression_core PROPERTIES CXX_EXTENSIONS OFF)

# The version string is injected by the build system so it has a single
# source of truth (the project() call above).
set_source_files_properties(src/Version.cpp PROPERTIES
    COMPILE_DEFINITIONS "EXPR_VERSION_STRING=\"${PROJECT_VERSION}\"")

expr_enable_warnings(expression_core)

# ---------------------------------------------------------------------------
# Sub-projects
# ---------------------------------------------------------------------------
if(EXPR_BUILD_APP)
    add_subdirectory(app)
endif()

if(EXPR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
