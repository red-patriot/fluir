#include <filesystem>
#include <fstream>
#include <iostream>

import fluir.frontend;
import fluir.backend;
import fluir.utility.context;
import fluir.utility.pass;

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
  auto frontendResults = fluir::addContext(fluir::Context{}, source) | fluir::parseFile | fluir::buildGraph;
  printDiagnostics(frontendResults.ctx.diagnostics);
  if (frontendResults.ctx.diagnostics.containsErrors()) {
    return 1;
  }

  auto backendResults = frontendResults | fluir::generateCode;
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
