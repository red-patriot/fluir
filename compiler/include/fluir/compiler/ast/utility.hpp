#ifndef FLUIR_COMPILER_AST_UTILITY_HPP
#define FLUIR_COMPILER_AST_UTILITY_HPP

#include <cstddef>

namespace fluir {
  using id_t = std::size_t;

  namespace ast {
    struct LocationInfo {
      std::ptrdiff_t x;
      std::ptrdiff_t y;
      std::ptrdiff_t z;
      size_t width;
      size_t height;

      friend bool operator==(const LocationInfo&, const LocationInfo&) = default;
      friend bool operator!=(const LocationInfo&, const LocationInfo&) = default;
    };
  }  // namespace ast
}  // namespace fluir

#endif
