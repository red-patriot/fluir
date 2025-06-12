module;

#include <string>

#include "bytecode/byte_code.hpp"

export module fluir.decoder;

namespace fluir {
  export code::ByteCode decode(std::string_view source);
}  // namespace fluir
