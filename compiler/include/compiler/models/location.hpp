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
}  // namespace fluir

#endif
