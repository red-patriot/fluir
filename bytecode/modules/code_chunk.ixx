module;

#include <string>
#include <vector>
#include <cstdint>

export module fluir.code_chunk;
export import fluir.instruction;

namespace fluir::code {
  export using Value = double;  // TODO: Support other value types
  export using Bytes = std::vector<uint8_t>;

  export struct Chunk {
    std::string name = "";
    Bytes code{};
    std::vector<Value> constants{};
  };
}  // namespace fluir::code
