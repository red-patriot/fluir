#ifndef FLUIR_BYTECODE_CODE_CHUNK_HPP
#define FLUIR_BYTECODE_CODE_CHUNK_HPP

#include <vector>

#include "instruction.hpp"

namespace fluir::code {
  using Value = double;  // TODO: Support other value types

  struct Chunk {
    std::vector<uint8_t> code;
    std::vector<Value> constants;
  };
}  // namespace fluir::code

#endif
