#ifndef FLUIR_VM_VALIDATE_HPP
#define FLUIR_VM_VALIDATE_HPP

#include <bytecode/version.hpp>

namespace fluir {
  bool checkVersionIsValid(const Version& vmVersion, const Version& bytecodeVersion);
}

#endif //VALIDATE_HPP
