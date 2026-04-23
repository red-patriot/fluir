#ifndef FLUIR_COMPILER_MODELS_LOCATION_HPP
#define FLUIR_COMPILER_MODELS_LOCATION_HPP

namespace fluir {
  struct FlowGraphLocation {
    int x;
    int y;
    int z;
    int width;
    int height;

    friend bool operator==(const FlowGraphLocation&, const FlowGraphLocation&) = default;
  };

  struct Coordinate {
    int x;
    int y;
    int z;
    friend bool operator==(const Coordinate&, const Coordinate&) = default;
  };

  struct Size {
    int width;
    int height;
    friend bool operator==(const Size&, const Size&) = default;
  };
}  // namespace fluir

#endif
