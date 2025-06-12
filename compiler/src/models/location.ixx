export module fluir.models.location;

namespace fluir {
  export struct FlowGraphLocation {
    int x;
    int y;
    int z;
    int width;
    int height;

    friend bool operator==(const FlowGraphLocation&, const FlowGraphLocation&) = default;
  };
}  // namespace fluir
