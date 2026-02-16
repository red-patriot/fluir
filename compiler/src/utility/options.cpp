#include "compiler/utility/options.hpp"

#include <algorithm>
#include <cerrno>
#include <iostream>

#include <argparse/argparse.hpp>
#include <bytecode/version.hpp>
#include <fmt/format.h>

using namespace std::string_literals;

namespace fluir {
  std::optional<CompilerOptions> parseArgs(int argc, const char** argv) {
    argparse::ArgumentParser parser(
      "Fluir compiler",
      fmt::format(
        "ALPHA {}.{}.{}", fluir::CURRENT_VERSION.major, fluir::CURRENT_VERSION.minor, fluir::CURRENT_VERSION.patch));

    parser.add_description(
      "The compiler for the Fluir programming language.\n"
      "NOTE:\n"
      "\tThe Fluir language is currently in ALPHA, as such so guarantees are made for backwards-compatibility.\n"
      "\tThere are likely bugs. If you find a bug, please submit an issue on github.");

    parser.add_argument("file").help("The path to the fluir source file to compile.").required();

    parser.add_argument("--output", "-o").nargs(1).help("The file path to write the output.").default_value("out.flc"s);
    parser.add_argument("--no-color").help("Disables printing colored output.").flag();

    // Add hidden developer arguments
    parser.add_argument("--dev-no-version").hidden().help("Disables checking the version of the input file.").flag();
    parser.add_argument("--dev-print-ast").hidden().help("Prints the AST to standard out after parsing.").flag();

    try {
      parser.parse_args(argc, argv);
    } catch (const std::runtime_error&) {
      std::cout << parser << '\n';
      return std::nullopt;
    }

    return CompilerOptions{
      .inputFilename = parser.get<std::string>("file"),
      .outputFilename = parser.get<std::string>("--output"),
      .colorOutput = !parser.get<bool>("--no-color"),
      .developerOptions = DeveloperOptions{.printAST = parser.get<bool>("--dev-print-ast"),
                                           .suppressVersionErrors = parser.get<bool>("--dev-no-version")}};
  }
  std::optional<CompilerOptions> parseArgs(const std::vector<std::string>& args) {
    int argc = static_cast<int>(args.size());
    std::vector<const char*> argv;
    std::ranges::transform(args, std::back_inserter(argv), [](const auto& arg) { return arg.data(); });

    return parseArgs(argc, argv.data());
  }
}  // namespace fluir
