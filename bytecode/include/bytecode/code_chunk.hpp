#ifndef FLUIR_BYTECODE_CODE_CHUNK_HPP
#define FLUIR_BYTECODE_CODE_CHUNK_HPP

#include <vector>

#include "op_code.hpp"

namespace fluir {
  struct CodeChunk {
    std::vector<OpCode> code;
  };
}  // namespace fluir

#endif
