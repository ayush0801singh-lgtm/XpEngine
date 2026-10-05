# Interview Notes

This document provides a technical walkthrough of the `expression-engine` project, highlighting the C++ engineering decisions, algorithmic complexities, and design patterns. It is intended as a guide to defend the project in a Software Engineering interview (e.g. for Siemens, Google, or Bloomberg).

## 1. Why a Lexer is needed
If we tried to parse a string character-by-character, we would constantly have to handle whitespace, multi-digit numbers, and multi-character functions (like `sin`). The Lexer consumes the raw string and groups characters into atomic `Token` objects (e.g., `TokenType::Number` or `TokenType::Identifier`). This means the parser only has to deal with a clean stream of semantic tokens, drastically simplifying the parser's logic. 

**Memory Tradeoff:** Tokens store a copy of their `lexeme` (a `std::string`) rather than a `std::string_view`. This forces a small heap allocation per token, but eliminates lifetime dependencies, making the token stream fully independent of the original input string. This is a deliberate "Safety / Ease of Use vs Raw Performance" tradeoff.

## 2. Why parsing is separated from evaluation
Coupling parsing and evaluation (e.g., evaluating math *while* reading the string) is fragile and restricts functionality. By producing an Abstract Syntax Tree (AST), the parser acts purely as a syntax validator. The AST can then be evaluated once, evaluated a million times in a loop (caching the parsing cost), optimized, or printed.

## 3. Why an AST is useful
An AST encodes the mathematical structure in memory. `(3 + 4) * 2` becomes a tree where `*` is the root, `2` is the right child, and `+` (with `3` and `4`) is the left child. Parentheses completely disappear from the AST; they only exist to guide the tree's construction.

## 4. How the Shunting-Yard algorithm works
Dijkstra's Shunting-Yard algorithm converts infix expressions (`3 + 4`) into a structure respecting operator precedence without using recursive descent.
In our implementation, it maintains two stacks:
- **Operands stack:** Subtrees (AST nodes) waiting to be consumed.
- **Operators stack:** Unresolved operators waiting for their right-hand sides.

The algorithm uses a state machine (`expectOperand_`) to differentiate unary from binary operators (e.g., whether `-` means negative or subtraction). If we expect an operand and see `-`, it's unary. If we expect an operator and see `-`, it's binary. This state machine also guarantees that 99% of syntax errors (like `3 + * 4`) are caught immediately before they corrupt the stacks.

## 5. Operator precedence and associativity
Operators are defined in `OperatorTable.cpp`. 
- **Precedence:** Determines which operator binds tighter. `*` has higher precedence than `+`, so `3 + 4 * 2` groups as `3 + (4 * 2)`.
- **Associativity:** Determines grouping for operators of the *same* precedence. `+` is Left-Associative (`1 + 2 + 3` is `(1 + 2) + 3`), while `^` (power) is Right-Associative (`2 ^ 3 ^ 2` is `2 ^ (3 ^ 2) = 2^9 = 512`). 
The `ShuntingYardParser::pushBinaryOperator` uses these rules to decide when to pop and evaluate pending operators from the stack.

## 6. How unary minus is handled
Unary operators (`-5`) act as prefix modifiers. When the parser is expecting an operand and encounters a `-`, it treats it as a unary minus, pushing it to the operator stack. Unlike binary operators, prefix operators have no left-hand side, so they don't force earlier operators to resolve.

## 7. Why `std::unique_ptr` is used
The AST is a recursive polymorphic data structure (e.g., a `BinaryOpNode` holds two `ASTNode` children). We cannot store subclasses by value because of object slicing, and we cannot use `sizeof(ASTNode)` if it holds itself. We use `std::unique_ptr<ASTNode>` because an AST node strictly and exclusively *owns* its children.

## 8. RAII and ownership decisions
Using `std::unique_ptr` means we get Memory Safety / RAII for free. When the root node of the AST goes out of scope, its destructor destroys the `unique_ptr`s of its children, which destroys their children, cascading down the tree. There is not a single manual `delete` in the entire codebase, effectively eliminating memory leak vectors.

## 9. Error handling decisions
Exceptions (`ExpressionError`, `LexError`, `ParseError`, `EvaluationError`) are used strictly *internally* to break out of deep call stacks (like recursive AST evaluation).
At the API boundary (`Engine::evaluate`), these exceptions are caught and wrapped in a `expr::Result` object (modeled after C++23 `std::expected` but implemented via C++17 `std::variant`). This ensures the library is polite, non-terminating, and fully safe for end-users, while preserving the exact column position of the error.

## 10. Time and space complexity
- **Lexer:** 
  - Time: `O(N)` where `N` is the length of the string. 
  - Space: `O(T)` where `T` is the number of tokens (for the vector).
- **Parser:** 
  - Time: `O(T)`. Each token is pushed and popped at most once.
  - Space: `O(T)` for the operator/operand stacks and the resulting AST.
- **AST Evaluation:** 
  - Time: `O(V)` where `V` is the number of nodes in the AST. 
  - Space: `O(D)` where `D` is the maximum depth of the AST (for the C++ call stack during the Visitor traversal).

## 11. Important design tradeoffs
- **Virtual Dispatch vs `std::variant` for AST Nodes:** We used classical OOP polymorphism (the Visitor pattern) for the AST instead of `std::variant` + `std::visit`. While `std::variant` can be slightly faster due to better cache locality and devirtualization, it makes the AST recursive definition much harder to read in C++17. The OOP approach is standard, highly extensible, and very easy to explain.

## 12. Potential bottlenecks
- `std::unordered_map` variable lookup: A hash map lookup per variable access is fast, but for a high-performance interpreter, we would resolve variable names to integer indices during an AST-optimization pass, and use a raw `std::vector` for environment lookup at runtime.

## 13. How the project could be extended
Adding an `AssignmentNode` to the parser (to natively support `x = 5 + 2`) or adding boolean logic (`>`, `<`, `==`, `if()`). The Visitor pattern makes this trivial: just add the nodes, add them to `ASTVisitor`, and implement the evaluation logic in `EvaluatorVisitor`.

## 14. Likely interview questions
**Q: "How do you avoid string allocations when querying the Environment?"**
A: By using a transparent comparator (`using is_transparent = void`) in our custom `StringHash` struct, C++14+ allows us to pass a `std::string_view` into `std::unordered_map::find`, finding the key without constructing a temporary `std::string`.

**Q: "Why does `ASTNode::accept` need to be implemented in *every* subclass? Why can't the base class do it?"**
A: If the base class implements `visitor.visit(*this);`, then `*this` resolves to `ASTNode&`, which defeats the point. By implementing it in the derived class, `*this` resolves to `BinaryOpNode&` (or whichever concrete class it is), allowing the compiler to statically pick the correct `visit()` overload on the Visitor via double-dispatch.
