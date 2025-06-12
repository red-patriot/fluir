#include <fstream>
#include <iostream>
#include <sstream>
#include <type_traits>

import fluir.vm;
import fluir.decoder;

int main(int argc, char** argv) {
  // TODO: Make this work better and add other flags
  if (argc != 2) {
    std::cerr << "Usage: fluir <file to execute>.\n";
    return -1;
  }

  std::ifstream fin(argv[1]);
  std::stringstream contents;
  contents << fin.rdbuf();

  auto bytecode = fluir::decode(contents.str());
  fluir::VirtualMachine vm;
  auto result = vm.execute(&bytecode);
  return static_cast<std::underlying_type_t<fluir::ExecResult>>(result);
}
