#include "editor/tools/grid_steps.hpp"

namespace fluir::editor {

  void GridSteps::start(Vec2 world) {
    last_ = world;
    remainder_ = Vec2{};
  }

  Vec2i GridSteps::advance(Vec2 world, double unit) {
    remainder_ = remainder_ + (world - last_);
    last_ = world;
    const Vec2i steps{static_cast<int>(remainder_.x / unit), static_cast<int>(remainder_.y / unit)};
    remainder_ = remainder_ - Vec2{steps.x * unit, steps.y * unit};
    return steps;
  }

}  // namespace fluir::editor
