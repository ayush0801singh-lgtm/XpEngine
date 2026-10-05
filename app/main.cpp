// Command-line front end for the expression engine.

#include "expression/Engine.hpp"
#include "expression/Version.hpp"

#include <iostream>
#include <string>
#include <string_view>

namespace {

void printUsage(std::string_view programName)
{
    std::cout << "Usage:\n"
              << "  " << programName << " \"<expression>\"   Evaluate a single expression\n"
              << "  " << programName << "                  Start interactive REPL\n"
              << "  " << programName << " --version        Show version information\n"
              << "  " << programName << " --help           Show this help message\n";
}

void printResult(const expr::Result& result, std::string_view source)
{
    if (result.hasError()) {
        const auto& error = result.error();
        std::cerr << error.message << '\n';
        if (error.position.has_value()) {
            const std::size_t pos = *error.position;
            std::cerr << source << '\n';
            std::cerr << std::string(pos, ' ') << "^\n";
        }
    } else {
        std::cout << result.value() << '\n';
    }
}

void runRepl()
{
    expr::Engine engine;
    std::string line;

    std::cout << "Expression Engine " << expr::version() << " REPL\n"
              << "Type an expression, or 'quit' / 'exit' to leave.\n";

    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) {
            break; // EOF
        }
        if (line.empty()) {
            continue;
        }
        if (line == "quit" || line == "exit") {
            break;
        }

        printResult(engine.evaluate(line), line);
    }
}

} // namespace

int main(int argc, char* argv[])
{
    const std::string_view programName = (argc > 0) ? argv[0] : "expression_engine";

    if (argc > 2) {
        std::cerr << "Error: Too many arguments.\n";
        printUsage(programName);
        return 1;
    }

    if (argc == 2) {
        const std::string_view arg = argv[1];
        if (arg == "--help" || arg == "-h") {
            printUsage(programName);
            return 0;
        }
        if (arg == "--version") {
            std::cout << "expression_engine " << expr::version() << '\n';
            return 0;
        }
        
        expr::Engine engine;
        printResult(engine.evaluate(arg), arg);
        return 0;
    }

    runRepl();
    return 0;
}
