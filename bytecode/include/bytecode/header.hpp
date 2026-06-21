#ifndef FLUIR_BYTECODE_HEADER_HPP
#define FLUIR_BYTECODE_HEADER_HPP

#include <cstdint>

namespace fluir::code {
  struct Header {
    char filetype{0};

    std::uint8_t major{0};
    std::uint8_t minor{0};
    std::uint8_t patch{0};

    std::uint64_t entryOffset{0};
  };
}  // namespace fluir::code

#endif
