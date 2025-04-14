#ifndef FLUIR_COMPILER_MODELS_LOCATION_HPP
#define FLUIR_COMPILER_MODELS_LOCATION_HPP

#include <cstdint>

namespace fluir {
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
