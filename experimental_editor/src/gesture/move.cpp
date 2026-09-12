#include "editor/gesture/move.hpp"

#include <memory>

#include "editor/transaction/move.hpp"

namespace fluir::editor {

  void MoveGesture::update(const EditorContext& ctx, Vec2 worldDelta) {
    delta_ = delta_ + accumulator_.fold(ctx.layout.unitPx, worldDelta);
  }

  FlowGraphLocation MoveGesture::preview(const FlowGraphLocation& loc) const {
    FlowGraphLocation out = loc;
    out.x += delta_.x;
    out.y += delta_.y;
    return out;
  }

  void MoveGesture::commit(const EditorContext& ctx, const FlowGraphLocation& loc, const fluir::FullID& id) {
    if (delta_ == Vec2i{}) {
      return;
    }
    ctx.dispatch(std::make_unique<MoveTransaction>(id, loc.x + delta_.x, loc.y + delta_.y));
  }

}  // namespace fluir::editor
