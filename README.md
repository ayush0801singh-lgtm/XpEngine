# Expression Parser & AST Evaluation Engine

A production-quality C++17 mathematical expression evaluator. It parses mathematical expressions into an Abstract Syntax Tree (AST) and evaluates them, featuring full support for variables, operator precedence, standard mathematical functions, and strict error handling.

## Architecture

The project is strictly separated into four phases: Lexing, Parsing, AST processing, and Evaluation. The public `Engine` facade wraps these steps, providing a safe, non-throwing API.

```text
       Input String ("3 + 4 * 2")
             │
             ▼
       Lexer / Tokenizer
             │ (std::vector<Token>)
             ▼
       Shunting-Yard Parser
             │ (std::unique_ptr<ASTNode>)
             ▼
       Abstract Syntax Tree
             │ (Visitor Pattern)
             ▼
       Evaluator + Environment
             │
             ▼
       Result (double / Error)
```

## Features

- **Variables:** `x = 10`, `x * 2` (Environment state persists between calls)
- **Math functions:** `sin`, `cos`, `tan`, `sqrt`, `log`, `exp`, `abs`, `min`, `max`
- **Operators:** `+`, `-`, `*`, `/`, `%`, `^`, unary `+`, unary `-`
- **Rigorous Error Handling:**
  - Reports exact character offset of lexical errors, syntax errors, and runtime evaluation errors.
  - Safely catches division by zero, missing arguments, undefined variables, and mathematical domain errors (e.g. `sqrt(-1)`).
- **Interactive REPL:** A Read-Eval-Print-Loop application to test expressions interactively.

## Build Instructions

Requires a C++17 compiler and CMake 3.14+.

### Windows (MSYS2 MinGW-w64) / Linux (GCC/Clang) / macOS
```bash
# Generate build files
cmake -B build -S .

# Build the project
cmake --build build

# Run unit tests and E2E integration tests
ctest --test-dir build -j8 -V
```

## Usage

### Command Line Interface
Evaluate a single expression:
```bash
$ ./build/app/expression_engine "3 + 4 * 2"
11
$ ./build/app/expression_engine "5 / 0"
Evaluation error at column 3: division by zero
5 / 0
  ^
```

Start the interactive REPL:
```bash
$ ./build/app/expression_engine
Expression Engine REPL
Type an expression, or 'quit' / 'exit' to leave.
> x = 5
5
> x * 2
10
> quit
```

### Library API
The library is designed to be easily embedded in other applications via the `expr::Engine` facade.

```cpp
#include "expression/Engine.hpp"
#include <iostream>

int main() {
    expr::Engine engine;
    
    // Set variables in the environment
    engine.environment().set("pi", 3.14159);
    
    // Evaluate expressions safely without try-catch blocks
    expr::Result result = engine.evaluate("sin(pi / 2)");
    
    if (result.hasError()) {
        std::cerr << "Error: " << result.error().message << "\n";
    } else {
        std::cout << "Result: " << result.value() << "\n";
    }
    
    return 0;
}
```

## Design Decisions

- **Why separate Parsing from Evaluation?** By keeping the parser ignorant of mathematical operations, we can extend the evaluator (e.g. adding new functions or variables) without touching the syntax rules. This also allows caching the compiled AST if we wanted to execute the same expression thousands of times in a loop.
- **Why a Shunting-Yard Parser instead of Regex?** Regular expressions cannot parse nested structures (like `(2 + (3 * 4))`). Shunting-Yard provides a mathematically proven way to handle infinite nesting, precedence, and associativity.
- **Why AST Nodes?** The AST makes the structure explicit. The Visitor pattern (via `ASTVisitor`) allows us to easily add an `ASTPrinter` or an `Evaluator` without modifying the node definitions themselves.
- **Why C++17?** C++17 features like `std::optional`, `std::string_view`, and `std::variant` are critical for building safe, zero-allocation APIs. Specifically, we used `std::variant` to emulate modern `std::expected` for error handling.

## Future Improvements

1. **JIT Compilation:** The AST could be compiled to LLVM IR or raw x64 machine code instead of interpreted via Visitor, yielding a massive performance boost for looping.
2. **Assignments inside expressions:** Expand the parser to natively understand `x = 5` instead of only supporting it manually via `Environment::set`. (Currently, `x = 5` works in the REPL via standard parsing, wait—actually the REPL doesn't parse `=` yet, we'd add an AssignmentNode for that!).
3. **Type System:** Extend the engine to handle strings and booleans (e.g., `if(x > 5, 1, 0)`).
