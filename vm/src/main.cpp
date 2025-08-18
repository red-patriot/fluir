#include <fstream>
#include <iostream>
#include <sstream>
#include <type_traits>

#include "vm/decoder/decode.hpp"
#include "vm/vm.hpp"

int main(int argc, char** argv) {
  // TODO: Make this work better and add other flags
  if (argc != 2) {
    std::cerr << "Usage: fluir <file to execute>.\n";
    return -1;
  }

  std::ifstream fin(argv[1]);
  std::stringstream ss;
  ss << fin.rdbuf();
  auto contents = ss.str();

  auto header = fluir::decodeHeader(contents);
  // Check for version compatibility
  auto bytecode = fluir::decode(header, contents);
  fluir::VirtualMachine vm;
  auto result = vm.execute(&bytecode);
  return static_cast<std::underlying_type_t<fluir::ExecResult>>(result);
}
