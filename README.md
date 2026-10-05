# Expression Parser & AST Evaluation Engine

A C++17 expression parsing and AST evaluation engine implementing lexical analysis, Shunting-Yard parsing, AST construction, and recursive evaluation. It provides a library interface for safe evaluation and a command-line REPL for interactive use.

## Architecture

The project is structured into four distinct phases, encapsulated by the `expr::Engine` facade:

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
- **Mathematical Functions:** `sin`, `cos`, `tan`, `sqrt`, `log`, `exp`, `abs`, `min`, `max`
- **Operators:** `+`, `-`, `*`, `/`, `%`, `^`, unary `+`, unary `-`
- **Error Handling:** Returns structured errors indicating the exact character offset of lexical errors, syntax errors, and runtime evaluation errors (e.g., division by zero or domain errors).
- **Interactive REPL:** A Read-Eval-Print-Loop application to test expressions interactively.

## Build Instructions

Requires a C++17 compiler and CMake 3.14+.

### Windows (MSYS2 MinGW-w64) / Linux (GCC/Clang) / macOS
```bash
# Generate build files
cmake -B build -S .

# Build the project
cmake --build build
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

The library is designed to be embedded in other applications via the `expr::Engine` facade.

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

## Testing

The project includes an extensive GoogleTest suite for unit tests and a Python script for End-to-End integration tests. 

To run the tests:
```bash
ctest --test-dir build -j8 -V
```

## Design Decisions

- **Separation of Parsing from Evaluation:** By producing an Abstract Syntax Tree (AST), the parser acts purely as a syntax validator. The AST can then be evaluated, printed, or processed independently.
- **Shunting-Yard Parser:** The parser uses Dijkstra's Shunting-Yard algorithm to handle infinite nesting, precedence, and associativity.
- **AST and Visitor Pattern:** The AST makes the syntactic structure explicit in memory using `std::unique_ptr` for exclusive ownership. The Visitor pattern (via `ASTVisitor`) is used to separate the evaluation logic from the node definitions.
- **C++17 Features:** `std::optional`, `std::string_view`, and `std::variant` are used to build safe, non-throwing APIs at the library boundary. `std::variant` is used specifically to emulate modern `std::expected` for error handling.

## Complexity Overview

- **Lexical Analysis:** `O(N)` time complexity (where `N` is string length) and `O(T)` space complexity (where `T` is token count).
- **Parsing:** `O(T)` time complexity (each token is pushed and popped at most once) and `O(T)` space complexity (for the stacks and AST).
- **Evaluation:** `O(V)` time complexity (where `V` is the number of AST nodes) and `O(D)` space complexity (where `D` is the AST depth) due to recursive traversal.

## Future Improvements

- **JIT Compilation:** Compiling the AST to bytecode or LLVM IR for improved performance in repeated evaluation loops.
- **Extended Parser:** Support for assignment expressions (`x = 5 + 2`) directly in the language grammar rather than manual environment injection.
- **Type System:** Extending the engine to handle strings, booleans, and control flow operators (`if`, `>`, `<`).
