#ifndef FLUIR_COMPILER_MODELS_ID_HPP
#define FLUIR_COMPILER_MODELS_ID_HPP

#include <cstdint>
#include <vector>

namespace fluir {
  using ID = std::uint64_t;
  using FullID = std::vector<ID>;
  constexpr ID INVALID_ID = 0;

  template <typename T>
  using WithID = std::pair<ID, T>;
}  // namespace fluir

#endif
