#include <fstream>
#include <iostream>
#include <sstream>
#include <type_traits>

#include <bytecode/version.hpp>

#include "vm/decoder/decode.hpp"
#include "vm/validate.hpp"
#include "vm/vm.hpp"

static std::ostream& operator<<(std::ostream& os, const fluir::Version& version);

int main(int argc, char** argv) {
  fluir::Version current;  // Default construct to get the current language version
  // TODO: Make this work better and add other flags
  if (argc != 2) {
    std::cerr << "Fluir version " << current << "\nUsage: fluir <file to execute>.\n";
    return -1;
  }

  std::ifstream fin(argv[1]);
  std::stringstream ss;
  ss << fin.rdbuf();
  auto contents = ss.str();

  auto header = fluir::decodeHeader(contents);
  if (fluir::Version fileVersion{header.major, header.minor, header.patch};
      !fluir::checkVersionIsValid(current, fileVersion)) {
    std::cerr << "Cannot run this bytecode. It has version " << fileVersion
              << " which cannot be run by a VM of version " << current << '\n';
    return -1;
  }

  auto bytecode = fluir::decode(header, contents);
  fluir::VirtualMachine vm;
  auto result = vm.execute(&bytecode);
  return static_cast<std::underlying_type_t<fluir::ExecResult>>(result);
}

static std::ostream& operator<<(std::ostream& os, const fluir::Version& version) {
  os << (int)version.major << '.' << (int)version.minor << '.' << (int)version.patch;
  return os;
}
