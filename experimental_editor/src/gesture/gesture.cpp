#include "editor/gesture/gesture.hpp"

namespace fluir::editor {

  Vec2i GridAccumulator::fold(double unitPx, Vec2 worldDelta) {
    remainder_ = remainder_ + worldDelta;
    const Vec2i steps{static_cast<int>(remainder_.x / unitPx), static_cast<int>(remainder_.y / unitPx)};
    remainder_.x -= steps.x * unitPx;
    remainder_.y -= steps.y * unitPx;
    return steps;
  }

}  // namespace fluir::editor
