module;

#include <cstdint>
#include <vector>

export module fluir.byte_code;
export import fluir.code_chunk;

namespace fluir::code {
  export struct Header {
    char filetype{0};

    std::uint8_t major{0};
    std::uint8_t minor{0};
    std::uint8_t patch{0};

    std::uint64_t entryOffset{0};
  };

  export struct ByteCode {
    Header header{};
    std::vector<Chunk> chunks{};
  };
}  // namespace fluir::code
