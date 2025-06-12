module;

#include <cstdint>

export module fluir.models.id;

namespace fluir {
  export using ID = std::uint64_t;
  export constexpr ID INVALID_ID = 0;
}  // namespace fluir
