#ifndef FLUIR_VM_DECODE_HPP
#define FLUIR_VM_DECODE_HPP

#include <string>

#include "bytecode/byte_code.hpp"

namespace fluir {
  code::Header decodeHeader(std::string_view source);
  code::ByteCode decode(std::string_view source);
}  // namespace fluir

#endif
