#include "editor/components/resize_handle.hpp"

#include <memory>

#include "editor/core/renderer.hpp"
#include "editor/transaction/resize.hpp"

namespace fluir::editor {
  namespace {
    Rect handleBox(const Rect& nodeRect, const FlowGraphLocation& loc, const EditorContext& ctx) {
      const Rect box = resizeRect(loc);
      const auto u = ctx.layout.unitPx;
      return {nodeRect.x + box.x * u, nodeRect.y + box.y * u, box.w * u, box.h * u};
    }
  }  // namespace

  Rect resizeRect(const fluir::FlowGraphLocation& nodeLoc) {
    return Rect{.x = nodeLoc.width - ResizeHandle::WIDTH,
                .y = 0,
                .w = ResizeHandle::WIDTH,
                .h = static_cast<double>(nodeLoc.height)};
  }

  bool ResizeHandle::onDragStart(const EditorContext& ctx,
                                 Vec2 parentLocalPos,
                                 const FlowGraphLocation& loc,
                                 const Rect& nodeRect) {
    accumulator_ = 0;
    dw_ = 0;
    active_ = handleBox(nodeRect, loc, ctx).contains(parentLocalPos);
    return active_;
  }

  void ResizeHandle::onDrag(const EditorContext& ctx, Vec2 worldDelta) {
    if (!active_) {
      return;
    }
    accumulator_ += worldDelta.x;
    const double unit = ctx.layout.unitPx;
    const int dw = static_cast<int>(accumulator_ / unit);
    if (dw == 0) {
      return;
    }
    accumulator_ -= dw * unit;  // keep only the sub-unit remainder
    dw_ += dw;
  }

  FlowGraphLocation ResizeHandle::preview(const FlowGraphLocation& loc) const {
    FlowGraphLocation out = loc;
    out.width = clampSize(loc.width + dw_);
    return out;
  }

  void ResizeHandle::commit(const EditorContext& ctx, const FlowGraphLocation& loc, const fluir::FullID& id) {
    if (active_) {
      const int width = preview(loc).width;
      if (width != loc.width) {
        ctx.dispatch(std::make_unique<ResizeTransaction>(id, width, loc.height));
      }
    }
    cancel();
  }

  void ResizeHandle::cancel() {
    accumulator_ = 0;
    dw_ = 0;
    active_ = false;
  }

  void ResizeHandle::draw(const Subview& view,
                          const EditorContext& ctx,
                          const FlowGraphLocation& loc,
                          const Rect& nodeRect) const {
    view.renderer().fillRect(view.toScreen(handleBox(nodeRect, loc, ctx)), ctx.theme.border);
  }

}  // namespace fluir::editor
