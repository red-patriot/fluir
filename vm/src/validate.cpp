#include "vm/validate.hpp"

namespace fluir {
  bool checkVersionIsValid(const Version& vmVersion, const Version& bytecodeVersion) {
    return vmVersion >= bytecodeVersion;
  }
}