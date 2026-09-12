#include "editor/components/resize_handle.hpp"

#include <memory>

#include "editor/core/renderer.hpp"
#include "editor/transaction/resize.hpp"

namespace fluir::editor {
  namespace {
    Rect handleBox(const Rect& nodeRect, const Rect& box, const EditorContext& ctx) {
      const auto u = ctx.layout.unitPx;
      return {nodeRect.x + box.x * u, nodeRect.y + box.y * u, box.w * u, box.h * u};
    }
  }  // namespace

  bool HorizResizeHandle::onDragStart(const EditorContext& ctx,
                                      Vec2 parentLocalPos,
                                      const FlowGraphLocation& loc,
                                      const Rect& nodeRect) {
    accumulator_ = 0;
    dw_ = 0;
    active_ = handleBox(nodeRect, rect(loc), ctx).contains(parentLocalPos);
    return active_;
  }

  void HorizResizeHandle::onDrag(const EditorContext& ctx, Vec2 worldDelta) {
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

  FlowGraphLocation HorizResizeHandle::preview(const FlowGraphLocation& loc) const {
    FlowGraphLocation out = loc;
    out.width = std::clamp(loc.width + dw_, limits_.lower, limits_.upper);
    return out;
  }

  void HorizResizeHandle::commit(const EditorContext& ctx, const FlowGraphLocation& loc, const fluir::FullID& id) {
    if (active_) {
      const int width = preview(loc).width;
      if (width != loc.width) {
        ctx.dispatch(std::make_unique<ResizeTransaction>(id, width, loc.height));
      }
    }
    cancel();
  }

  void HorizResizeHandle::cancel() {
    accumulator_ = 0;
    dw_ = 0;
    active_ = false;
  }

  void HorizResizeHandle::draw(const Subview& view,
                               const EditorContext& ctx,
                               const FlowGraphLocation& loc,
                               const Rect& nodeRect) const {
    view.renderer().fillRect(view.toScreen(handleBox(nodeRect, rect(loc), ctx)), ctx.theme.border);
  }

  Rect HorizResizeHandle::rect(const fluir::FlowGraphLocation& nodeLoc) const {
    return Rect{.x = nodeLoc.width - WIDTH, .y = 0, .w = WIDTH, .h = static_cast<double>(nodeLoc.height)};
  }

  bool XYResizeHandle::onDragStart(const EditorContext& ctx,
                                   Vec2 parentLocalPos,
                                   const FlowGraphLocation& loc,
                                   const Rect& nodeRect) {
    accumulator_ = {0.0, 0.0};
    dw_ = 0;
    dh_ = 0;
    active_ = handleBox(nodeRect, rect(loc), ctx).contains(parentLocalPos);
    return active_;
  }

  void XYResizeHandle::onDrag(const EditorContext& ctx, Vec2 worldDelta) {
    if (!active_) {
      return;
    }
    accumulator_ = accumulator_ + worldDelta;
    const double unit = ctx.layout.unitPx;
    const int dw = static_cast<int>(accumulator_.x / unit);
    const int dh = static_cast<int>(accumulator_.y / unit);
    if (dw == 0 && dh == 0) {
      return;
    }
    accumulator_.x -= dw * unit;  // keep only the sub-unit remainder
    accumulator_.y -= dh * unit;
    dw_ += dw;
    dh_ += dh;
  }

  FlowGraphLocation XYResizeHandle::preview(const FlowGraphLocation& loc) const {
    FlowGraphLocation out = loc;
    // TODO: Vec2 is a template
    out.width = std::clamp(loc.width + dw_, (int)limits_.lower.x, (int)limits_.upper.x);
    out.height = std::clamp(loc.height + dh_, (int)limits_.lower.y, (int)limits_.upper.y);
    return out;
  }

  void XYResizeHandle::commit(const EditorContext& ctx, const FlowGraphLocation& loc, const fluir::FullID& id) {
    if (active_) {
      const int width = preview(loc).width;
      const int height = preview(loc).height;
      if (width != loc.width || height != loc.height) {
        ctx.dispatch(std::make_unique<ResizeTransaction>(id, width, height));
      }
    }
    cancel();
  }

  void XYResizeHandle::cancel() {
    accumulator_ = {0.0, 0.0};
    dw_ = 0;
    dh_ = 0;
    active_ = false;
  }

  void XYResizeHandle::draw(const Subview& view,
                            const EditorContext& ctx,
                            const FlowGraphLocation& loc,
                            const Rect& nodeRect) const {
    view.renderer().fillRect(view.toScreen(handleBox(nodeRect, rect(loc), ctx)), ctx.theme.border);
  }

  Rect XYResizeHandle::rect(const fluir::FlowGraphLocation& nodeLoc) const {
    return Rect{.x = nodeLoc.width - SIZE, .y = nodeLoc.height - SIZE, .w = SIZE, .h = SIZE};
  }

}  // namespace fluir::editor
