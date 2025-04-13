#ifndef FLUIR_COMPILER_PARSE_TREE_UTILITY_HPP
#define FLUIR_COMPILER_PARSE_TREE_UTILITY_HPP

#include <cstddef>

namespace fluir {
  using id_t = std::uint64_t;
  constexpr id_t INVALID_ID = 0;

  struct LocationInfo {
    int x;
    int y;
    int z;
    int width;
    int height;

    friend bool operator==(const LocationInfo&, const LocationInfo&) = default;
    friend bool operator!=(const LocationInfo&, const LocationInfo&) = default;
  };
}  // namespace fluir

#endif
