#ifndef FLUIR_VM_CODE_BUILTIN_HPP
#define FLUIR_VM_CODE_BUILTIN_HPP

#include <ostream>

#include "vm/code/native_func.hpp"

namespace fluir {
  [[nodiscard]] NativeFunctionsMap getBuiltins();

  /** Prints the value on the top of the call frame and pops it*/
  void print(CallFrame& frame);

  std::ostream& operator<<(std::ostream& os, const code::Value& value);
}  // namespace fluir
#endif
