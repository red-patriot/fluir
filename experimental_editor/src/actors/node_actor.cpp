#include "editor/actors/node_actor.hpp"

#include <algorithm>

#include "editor/core/renderer.hpp"

namespace fluir::editor {

  namespace {

    constexpr double kDragHandleUnits = 3.0;  // handle square side, in grid units

    // The handle is a square `kDragHandleUnits` wide, clamped so it never
    // overflows a node smaller than that in either axis.
    double handleSpan(const EditorContext& ctx, double w, double h) {
      return std::min({kDragHandleUnits * ctx.layout.unitPx, w, h});
    }

  }  // namespace

  bool NodeActor::onDragStart(const EditorContext& ctx, Vec2 worldPos) {
    dragAccum_ = {};
    return dragHandleWorldRect(ctx).contains(worldPos);
  }

  void NodeActor::onDrag(const EditorContext& ctx, Vec2 /*worldPos*/, Vec2 worldDelta) {
    dragAccum_ = dragAccum_ + worldDelta;

    const double u = ctx.layout.unitPx;
    const int dx = static_cast<int>(dragAccum_.x / u);  // truncates toward zero
    const int dy = static_cast<int>(dragAccum_.y / u);
    if (dx == 0 && dy == 0) {
      return;
    }

    dragAccum_.x -= dx * u;
    dragAccum_.y -= dy * u;
    nudgeLocation(dx, dy);

    const Rect b = bounds();
    setBounds({b.x + dx * u, b.y + dy * u, b.w, b.h});
  }

  Rect NodeActor::dragHandleWorldRect(const EditorContext& ctx) const {
    const Rect b = bounds();
    const double s = handleSpan(ctx, b.w, b.h);
    return {b.x, b.y, s, s};
  }

  void NodeActor::drawDragHandle(const Subview& body, const EditorContext& ctx, Rect nodeRect) const {
    const double s = handleSpan(ctx, nodeRect.w, nodeRect.h);
    body.renderer().fillRect(body.toScreen(Rect{nodeRect.x, nodeRect.y, s, s}), ctx.theme.border);
  }

}  // namespace fluir::editor
