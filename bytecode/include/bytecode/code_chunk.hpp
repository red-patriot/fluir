#ifndef FLUIR_BYTECODE_CODE_CHUNK_HPP
#define FLUIR_BYTECODE_CODE_CHUNK_HPP

#include <string>
#include <vector>

#include "instruction.hpp"
#include "value.hpp"

namespace fluir::code {
  using Bytes = std::vector<uint8_t>;

  struct Chunk {
    std::string name = "";
    Bytes code{};
    std::vector<Value> constants{};
    std::uint8_t inCount{0};
    std::uint8_t outCount{0};
  };
}  // namespace fluir::code

#endif
