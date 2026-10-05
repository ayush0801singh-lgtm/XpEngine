// Command-line front end for the expression engine.
//
// Phase 1: skeleton only. Parsing/evaluation is added in later phases; this
// file will stay a thin layer over the expr::core library.

#include "expression/Version.hpp"

#include <iostream>
#include <string_view>

namespace {

void printUsage(std::string_view programName)
{
    std::cout << "Usage:\n"
              << "  " << programName << " \"<expression>\"\n"
              << "  " << programName << " --version\n";
}

} // namespace

int main(int argc, char* argv[])
{
    const std::string_view programName = (argc > 0) ? argv[0] : "expression_engine";

    if (argc == 2 && std::string_view(argv[1]) == "--version") {
        std::cout << "expression_engine " << expr::version() << '\n';
        return 0;
    }

    printUsage(programName);
    std::cout << "\n(Expression evaluation is not implemented yet.)\n";
    return 0;
}
