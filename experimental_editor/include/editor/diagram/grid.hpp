#ifndef FLUIR_EDITOR_DIAGRAM_GRID_HPP
#define FLUIR_EDITOR_DIAGRAM_GRID_HPP

#include <algorithm>
#include <cmath>
#include <QPointF>

#include "editor/core/geometry.hpp"

namespace fluir::editor::diagram {

  /** Whole grid units covered from `from` to `to` (scene px), dropping any part unit. */
  inline Vec2i gridSteps(QPointF from, QPointF to, double unit) {
    const QPointF delta = (to - from) / unit;
    return {static_cast<int>(std::trunc(delta.x())), static_cast<int>(std::trunc(delta.y()))};
  }

  /** The grid point nearest `local`. */
  inline Vec2i nearestGridPoint(QPointF local, double unit) {
    return {static_cast<int>(std::lround(local.x() / unit)), static_cast<int>(std::lround(local.y() / unit))};
  }

  inline int clamp(int value, Limits<int> limits) { return std::clamp(value, limits.lower, limits.upper); }

}  // namespace fluir::editor::diagram

#endif
