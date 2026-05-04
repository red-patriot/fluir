#ifndef FLUIR_BYTECODE_BYTE_CODE_HPP
#define FLUIR_BYTECODE_BYTE_CODE_HPP

#include <vector>

#include "code_chunk.hpp"
#include "header.hpp"
#include "instruction.hpp"
#include "value.hpp"

namespace fluir::code {

  struct ByteCode {
    Header header{};
    std::vector<Value> constants{};
    std::vector<Chunk> chunks{};
  };
}  // namespace fluir::code

#endif
