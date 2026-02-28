#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <type_traits>

#include "../include/vm/machine/vm.hpp"
#include "bytecode/version.hpp"
#include "vm/decoder/decode.hpp"

bool checkVersion(const fluir::Version& codeVersion) {
  if (codeVersion != fluir::CURRENT_VERSION) {
    std::cerr << std::format("The bytecode version {}.{}.{} is not supported by this version of the VM.\n",
                             codeVersion.major,
                             codeVersion.minor,
                             codeVersion.patch);
    return false;
  }

  return true;
}

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
  if (!checkVersion({bytecode.header.major, bytecode.header.minor, bytecode.header.patch})) {
    return -2;
  }

  fluir::VirtualMachine vm;
  auto result = vm.execute(&bytecode);
  return static_cast<std::underlying_type_t<fluir::ExecResult>>(result);
}
