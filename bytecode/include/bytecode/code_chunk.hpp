#ifndef FLUIR_BYTECODE_CODE_CHUNK_HPP
#define FLUIR_BYTECODE_CODE_CHUNK_HPP

#include <string>
#include <vector>

#include "instruction.hpp"
#include "value.h"

namespace fluir::code {
  using Bytes = std::vector<uint8_t>;

  struct Chunk {
    std::string name = "";
    Bytes code{};
    std::vector<Value> constants{};
  };
}  // namespace fluir::code

#endif
