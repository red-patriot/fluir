#include "editor/core/viewport.hpp"

#include <algorithm>

namespace fluir::editor {

  Vec2 Viewport::worldToScreen(Vec2 world) const { return world * scale + pan; }

  Vec2 Viewport::screenToWorld(Vec2 screen) const { return (screen - pan) / scale; }

  void Viewport::zoomAbout(Vec2 screenPivot, double factor) {
    const Vec2 worldAtPivot = screenToWorld(screenPivot);
    scale *= factor;
    pan = screenPivot - worldAtPivot * scale;
  }

  void Viewport::fitRect(Rect world, Vec2 viewportSize) {
    if (world.w > 0.0 && world.h > 0.0) {
      scale = std::min(viewportSize.x / world.w, viewportSize.y / world.h);
    } else {
      scale = 1.0;
    }
    pan = viewportSize * 0.5 - world.center() * scale;
  }

}  // namespace fluir::editor
