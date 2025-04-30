#ifndef FLUIR_BYTECODE_BYTE_CODE_HPP
#define FLUIR_BYTECODE_BYTE_CODE_HPP

#include <cstdint>
#include <vector>

#include "code_chunk.hpp"
#include "instruction.hpp"

namespace fluir::code {
  struct Header {
    char filetype{0};

    std::uint8_t major{0};
    std::uint8_t minor{0};
    std::uint8_t patch{0};

    std::uint64_t entryOffset{0};
  };

  struct ByteCode {
    Header header{};
    std::vector<Chunk> chunks{};
  };
}  // namespace fluir::code

#endif
