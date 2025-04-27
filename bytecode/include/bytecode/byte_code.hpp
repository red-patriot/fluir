#ifndef FLUIR_BYTECODE_BYTE_CODE_HPP
#define FLUIR_BYTECODE_BYTE_CODE_HPP

#include <cstdint>

#include "code_chunk.hpp"
#include "op_code.hpp"

namespace fluir::code {
  struct Header {
    char filetype{0};

    std::uint8_t major{0};
    std::uint8_t minor{0};
    std::uint8_t patch{0};

    std::uint64_t entryOffset{0};
  };

  struct ByteCode {
    Header header;
  };
}  // namespace fluir::code

#endif
