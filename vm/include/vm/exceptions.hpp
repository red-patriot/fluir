#ifndef FLUIR_VM_EXCEPTIONS_HPP
#define FLUIR_VM_EXCEPTIONS_HPP

#include <stdexcept>

namespace fluir {
  struct VirtualMachineError : std::runtime_error {
    using std::runtime_error::runtime_error;
  };
}  // namespace fluir

#endif  // FLUIR_EXCEPTIONS_HPP
