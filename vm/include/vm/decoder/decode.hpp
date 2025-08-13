#ifndef FLUIR_VM_DECODE_HPP
#define FLUIR_VM_DECODE_HPP

#include <string>

#include "bytecode/byte_code.hpp"

namespace fluir {
  /** Decodes the header from a source string */
  code::Header decodeHeader(std::string_view source);
  /** Decodes a source string into runnable bytecode */
  code::ByteCode decode(std::string_view source);
}  // namespace fluir

#endif
