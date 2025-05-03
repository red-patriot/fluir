#include <filesystem>
#include <fstream>
#include <iostream>

#include "compiler/backend/bytecode_generator.hpp"
#include "compiler/backend/inspect_writer.hpp"
#include "compiler/frontend/asg_builder.hpp"
#include "compiler/frontend/parser.hpp"

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
  auto parseResults = fluir::parseFile(source);
  printDiagnostics(parseResults.diagnostics());
  if (parseResults.containsErrors()) {
    return 1;
  }

  auto asgResults = fluir::buildGraph(parseResults.value());
  printDiagnostics(asgResults.diagnostics());
  if (asgResults.containsErrors()) {
    return 1;
  }

  auto bytecodeResults = fluir::generateCode(asgResults.value());
  printDiagnostics(bytecodeResults.diagnostics());
  if (bytecodeResults.containsErrors()) {
    return 1;
  }

  {
    fs::path destination{"./out.flc"};
    std::ofstream fout{destination};
    fluir::InspectWriter writer{};
    fluir::writeCode(bytecodeResults.value(), writer, fout);
  }

  return 0;
}
