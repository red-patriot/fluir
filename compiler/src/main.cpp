#include <filesystem>
#include <fstream>
#include <iostream>

#include "compiler/backend/bytecode_generator.hpp"
#include "compiler/backend/inspect_writer.hpp"
#include "compiler/frontend/asg_builder.hpp"
#include "compiler/frontend/parser.hpp"
#include "compiler/frontend/type_checker.hpp"
#include "compiler/types/builtin_symbols.hpp"
#include "compiler/utility/context.hpp"
#include "compiler/utility/pass.hpp"

namespace fs = std::filesystem;

static void printDiagnostics(const fluir::Diagnostics& diagnostics) {
  for (const auto& diagnostic : diagnostics) {
    std::cout << fluir::toString(diagnostic) << '\n';
  }
}

int main(int argc, char** argv) {
  // TODO: Read real inputs from the command line
  if (argc != 2) {
    std::cerr << "Usage: fluir.compiler file.fl\n";
    return 1;
  }

  fs::path source = fs::canonical(fs::path{argv[1]});
  fluir::Context ctx;
  ctx.symbolTable = fluir::types::buildSymbolTable();

  auto frontendResults =
    fluir::addContext(std::move(ctx), source) | fluir::parseFile | fluir::buildGraph | fluir::typeCheck;
  printDiagnostics(frontendResults.ctx.diagnostics);
  if (frontendResults.ctx.diagnostics.containsErrors()) {
    return 1;
  }

  auto backendResults = std::move(frontendResults) | fluir::generateCode;
  printDiagnostics(backendResults.ctx.diagnostics);
  if (backendResults.ctx.diagnostics.containsErrors()) {
    return 1;
  }

  {
    fs::path destination{"./out.flc"};
    std::ofstream fout{destination};
    fluir::InspectWriter writer{};
    fluir::writeCode(backendResults.data.value(), writer, fout);
  }

  return 0;
}
