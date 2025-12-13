#include <filesystem>
#include <fstream>
#include <iostream>

#include "bytecode/version.hpp"
#include "compiler/backend/bytecode_generator.hpp"
#include "compiler/backend/inspect_writer.hpp"
#include "compiler/frontend/asg_builder.hpp"
#include "compiler/frontend/parser.hpp"
#include "compiler/frontend/type_checker.hpp"
#include "compiler/types/builtin_symbols.hpp"
#include "compiler/utility/context.hpp"
namespace fs = std::filesystem;

namespace {
  void printDiagnostics(const fluir::Diagnostics& diagnostics) {
    for (const auto& diagnostic : diagnostics) {
      std::cout << fluir::toString(diagnostic) << '\n';
    }
  }

  fluir::Results<fluir::asg::ASG> runFrontend(fluir::Context& ctx, const fs::path& source) {
    auto parseTree = fluir::parseFile(ctx, source);
    if (!parseTree) {
      return fluir::NoResult;
    }
    auto asg = fluir::buildGraph(ctx, *parseTree);
    if (!asg) {
      return fluir::NoResult;
    }
    asg = fluir::typeCheck(ctx, std::move(*asg));
    if (!asg) {
      return fluir::NoResult;
    }
    return asg;
  }
}  // namespace

int main(int argc, char** argv) {
  // TODO: Read real inputs from the command line
  try {
    if (argc != 2) {
      std::cerr << "Usage: fluir.compiler file.fl\n";
      return 1;
    }
    fs::path source;
    try {
      source = fs::canonical(fs::path{argv[1]});
    } catch (const std::filesystem::filesystem_error& e) {
      std::cerr << e.what() << '\n';
      return 1;
    }
    fluir::Context ctx{.version = fluir::CURRENT_VERSION};
    ctx.symbolTable = fluir::types::buildSymbolTable();

    auto asg = runFrontend(ctx, source);
    if (!asg) {
      return 1;
    }

    auto backendResults = fluir::generateCode(ctx, *asg);
    if (!backendResults) {
      return 1;
    }

    {
      fs::path destination{"./out.flc"};
      std::ofstream fout{destination};
      fluir::InspectWriter writer{};
      fluir::writeCode(*backendResults, writer, fout);
    }

    return 0;
  } catch (const std::exception& e) {
    std::cerr << "An internal error occurred:\n\t" << e.what() << '\n';
    return 1;
  }
}
