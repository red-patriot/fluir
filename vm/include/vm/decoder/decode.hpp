#ifndef FLUIR_VM_DECODE_HPP
#define FLUIR_VM_DECODE_HPP

#include <string>

#include "vm/code/byte_code.hpp"

namespace fluir {
  code::ByteCode decode(std::string_view source);
}  // namespace fluir

#endif
