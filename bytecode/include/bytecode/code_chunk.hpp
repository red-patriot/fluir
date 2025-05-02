#ifndef FLUIR_BYTECODE_CODE_CHUNK_HPP
#define FLUIR_BYTECODE_CODE_CHUNK_HPP

#include <string>
#include <vector>

#include "instruction.hpp"

namespace fluir::code {
  using Value = double;  // TODO: Support other value types
  using Bytes = std::vector<uint8_t>;

  struct Chunk {
    std::string name = "";
    Bytes code{};
    std::vector<Value> constants{};
  };
}  // namespace fluir::code

#endif
