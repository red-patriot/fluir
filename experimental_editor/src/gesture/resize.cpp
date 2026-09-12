#include "editor/gesture/resize.hpp"

#include <algorithm>
#include <memory>

#include "editor/transaction/resize.hpp"

namespace fluir::editor {

  void ResizeGesture::update(const EditorContext& ctx, Vec2 worldDelta) {
    delta_ = delta_ + accumulator_.fold(ctx.layout.unitPx, worldDelta);
  }

  FlowGraphLocation ResizeGesture::preview(const FlowGraphLocation& loc) const {
    FlowGraphLocation out = loc;
    out.width = std::clamp(loc.width + delta_.x, limits_.lower.x, limits_.upper.x);
    if (axes_ == Axes::XY) {
      out.height = std::clamp(loc.height + delta_.y, limits_.lower.y, limits_.upper.y);
    }
    return out;
  }

  void ResizeGesture::commit(const EditorContext& ctx, const FlowGraphLocation& loc, const fluir::FullID& id) {
    const FlowGraphLocation next = preview(loc);
    if (next.width == loc.width && next.height == loc.height) {
      return;
    }
    ctx.dispatch(std::make_unique<ResizeTransaction>(id, next.width, next.height));
  }

}  // namespace fluir::editor
