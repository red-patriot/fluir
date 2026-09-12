#include "editor/components/drag_handle.hpp"

#include <memory>

#include "editor/core/renderer.hpp"
#include "editor/transaction/move.hpp"

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
    dx_ = 0;
    dy_ = 0;
    active_ = handleBox(nodeRect, loc, ctx).contains(parentLocalPos);
    return active_;
  }

  void DragHandle::onDrag(const EditorContext& ctx, Vec2 worldDelta) {
    if (!active_) {
      return;
    }
    accumulator_ = accumulator_ + worldDelta;
    const double unit = ctx.layout.unitPx;
    const int dx = static_cast<int>(accumulator_.x / unit);
    const int dy = static_cast<int>(accumulator_.y / unit);
    if (dx == 0 && dy == 0) {
      return;
    }
    accumulator_.x -= dx * unit;  // keep only the sub-unit remainder
    accumulator_.y -= dy * unit;
    dx_ += dx;
    dy_ += dy;
  }

  FlowGraphLocation DragHandle::preview(const FlowGraphLocation& loc) const {
    FlowGraphLocation out = loc;
    out.x += dx_;
    out.y += dy_;
    return out;
  }

  void DragHandle::commit(const EditorContext& ctx, const FlowGraphLocation& loc, const fluir::FullID& id) {
    if (active_ && (dx_ != 0 || dy_ != 0)) {
      ctx.dispatch(std::make_unique<MoveTransaction>(id, loc.x + dx_, loc.y + dy_));
    }
    cancel();
  }

  void DragHandle::cancel() {
    accumulator_ = {};
    dx_ = 0;
    dy_ = 0;
    active_ = false;
  }

  void DragHandle::draw(const Subview& view,
                        const EditorContext& ctx,
                        const FlowGraphLocation& loc,
                        const Rect& nodeRect) const {
    view.renderer().fillRect(view.toScreen(handleBox(nodeRect, loc, ctx)), ctx.theme.border);
  }

}  // namespace fluir::editor
