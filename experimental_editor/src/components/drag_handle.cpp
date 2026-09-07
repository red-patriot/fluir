#include "editor/components/drag_handle.hpp"

#include "editor/core/renderer.hpp"

namespace fluir::editor {
  namespace {
    Rect handleBox(const Rect& nodeRect, const Rect& boxRect, const EditorContext& ctx) {
      const auto u = ctx.layout.unitPx;
      return {nodeRect.x + boxRect.x * u, nodeRect.y + boxRect.y * u, boxRect.w * u, boxRect.h * u};
    }
  }  // namespace

  DragHandle::DragHandle(Rect handleRect, FlowGraphLocation& location, Rect& bounds) :
    rect_(handleRect), location_(location), bounds_(bounds) { }

  bool DragHandle::onDragStart(const EditorContext& ctx, Vec2 worldPos) {
    accumulator_ = {};
    return handleBox(bounds_, rect_, ctx).contains(worldPos);
  }

  void DragHandle::onDrag(const EditorContext& ctx, Vec2 worldPos, Vec2 worldDelta) {
    accumulator_ = accumulator_ + worldDelta;
    const double unit = ctx.layout.unitPx;
    const int dx = static_cast<int>(accumulator_.x / unit);
    const int dy = static_cast<int>(accumulator_.y / unit);
    if (dx == 0 && dy == 0) {
      return;
    }
    accumulator_.x -= dx * unit;  // keep only the sub-unit remainder
    accumulator_.y -= dy * unit;
    location_.x += dx;
    location_.y += dy;
    bounds_.x += dx * unit;
    bounds_.y += dy * unit;  // move bounds by the same whole step
  }

  void DragHandle::draw(const Subview& view, const EditorContext& ctx, const Rect& nodeRect) const {
    view.renderer().fillRect(view.toScreen(handleBox(nodeRect, rect_, ctx)), ctx.theme.border);
  }

}  // namespace fluir::editor
