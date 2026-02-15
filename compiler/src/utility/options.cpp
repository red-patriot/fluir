#include "compiler/utility/options.hpp"

#include <algorithm>

#include <argparse/argparse.hpp>
#include <bytecode/version.hpp>
#include <fmt/format.h>

using namespace std::string_literals;

namespace fluir {
  CompilerOptions parseArgs(int argc, const char** argv) {
    argparse::ArgumentParser parser(
      "Fluir compiler",
      fmt::format(
        "{}.{}.{}", fluir::CURRENT_VERSION.major, fluir::CURRENT_VERSION.minor, fluir::CURRENT_VERSION.patch));

    parser.add_description("The compiler for the Fluir programming language.");

    parser.add_argument("file").help("The path to the fluir source file to compile.").required();

    parser.add_argument("--output", "-o").help("The file path to write the output.").default_value("out.flc"s);
    parser.add_argument("--no-color")
      .help("Disables printing colored output.")
      .default_value(false)
      .implicit_value(true);

    parser.parse_args(argc, argv);

    return CompilerOptions{.inputFilename = parser.get<std::string>("file"),
                           .outputFilename = parser.get<std::string>("--output"),
                           .colorOutput = !parser.get<bool>("--no-color")};
  }
  CompilerOptions parseArgs(const std::vector<std::string>& args) {
    int argc = static_cast<int>(args.size());
    std::vector<const char*> argv;
    std::ranges::transform(args, std::back_inserter(argv), [](const auto& arg) { return arg.data(); });

    return parseArgs(argc, argv.data());
  }
}  // namespace fluir
