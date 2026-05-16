#ifndef FLUIR_VM_CODE_NATIVE_FUNC_HPP
#define FLUIR_VM_CODE_NATIVE_FUNC_HPP

#include <string_view>
#include <unordered_map>

#include "vm/machine/call_frame.hpp"

namespace fluir {
  typedef void (*NativeFunction)(CallFrame&);

  using NativeFunctionsMap = std::unordered_map<std::string_view, NativeFunction>;
}  // namespace fluir

#endif
