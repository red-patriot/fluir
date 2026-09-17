#include "editor/core/viewport.hpp"

#include <algorithm>

#include "editor/core/renderer.hpp"

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

  Subview::Subview(const Viewport& viewport, Rect screenClip, Renderer& renderer) :
    composed_{viewport}, renderer_(renderer) {
    renderer_.pushClip(screenClip);
  }

  Subview::Subview(const Subview& parent, Rect frameLocal) :
    composed_{parent.toScreen(frameLocal.topLeft()), parent.composed_.scale}, renderer_(parent.renderer_) {
    renderer_.pushClip(toScreen(Rect{0, 0, frameLocal.w, frameLocal.h}));
  }

  Subview::~Subview() { renderer_.popClip(); }

}  // namespace fluir::editor
