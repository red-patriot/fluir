#include "editor/components/drag_handle.hpp"

#include "editor/core/renderer.hpp"

namespace fluir::editor {
  namespace {
    Rect handleBox(const Rect& nodeRect, const FlowGraphLocation& loc, const EditorContext& ctx) {
      const Rect box = dragRect(loc);
      const auto u = ctx.layout.unitPx;
      return {nodeRect.x + box.x * u, nodeRect.y + box.y * u, box.w * u, box.h * u};
    }
  }  // namespace

  Rect dragRect(const fluir::FlowGraphLocation& nodeLoc) {
    return Rect{.x = nodeLoc.width - (DragHandle::WIDTH + 1), .y = 1, .w = DragHandle::WIDTH, .h = DragHandle::HEIGHT};
  }

  bool DragHandle::onDragStart(const EditorContext& ctx,
                               Vec2 parentLocalPos,
                               const FlowGraphLocation& loc,
                               const Rect& nodeRect) {
    accumulator_ = {};
    return handleBox(nodeRect, loc, ctx).contains(parentLocalPos);
  }

  void DragHandle::onDrag(const EditorContext& ctx, Vec2 worldDelta, FlowGraphLocation& loc) {
    accumulator_ = accumulator_ + worldDelta;
    const double unit = ctx.layout.unitPx;
    const int dx = static_cast<int>(accumulator_.x / unit);
    const int dy = static_cast<int>(accumulator_.y / unit);
    if (dx == 0 && dy == 0) {
      return;
    }
    accumulator_.x -= dx * unit;  // keep only the sub-unit remainder
    accumulator_.y -= dy * unit;
    loc.x += dx;
    loc.y += dy;
  }

  void DragHandle::draw(const Subview& view,
                        const EditorContext& ctx,
                        const FlowGraphLocation& loc,
                        const Rect& nodeRect) const {
    view.renderer().fillRect(view.toScreen(handleBox(nodeRect, loc, ctx)), ctx.theme.border);
  }

}  // namespace fluir::editor
