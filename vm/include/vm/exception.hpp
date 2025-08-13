#ifndef FLUIR_VM_EXCEPTION_HPP
#define FLUIR_VM_EXCEPTION_HPP

#include <stdexcept>

namespace fluir {
  struct InvalidBytecodeFile : std::runtime_error {
    using std::runtime_error::runtime_error;
  };
}

#endif //EXCEPTION_HPP
