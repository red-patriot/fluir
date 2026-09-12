#include "editor/actors/node_actor.hpp"

#include "editor/core/graph_geometry.hpp"

namespace fluir::editor {

  void NodeActor::layout(const EditorContext& ctx) {
    setBounds(localRect(previewLocation(), ctx.layout.unitPx));
    Actor::layout(ctx);
  }

  void NodeActor::drawHandles(const Subview& view, const EditorContext& ctx, const Rect& nodeRect) const {
    const FlowGraphLocation loc = previewLocation();
    drag_.draw(view, ctx, loc, nodeRect);
    resize_.draw(view, ctx, loc, nodeRect);
  }

  bool NodeActor::onDragStart(const EditorContext& ctx, Vec2 position) {
    const FlowGraphLocation loc = previewLocation();
    // Short-circuit: the drag grip keeps every pixel it shares with the bar.
    return drag_.onDragStart(ctx, position, loc, bounds()) || resize_.onDragStart(ctx, position, loc, bounds());
  }

  void NodeActor::onDrag(const EditorContext& ctx, Vec2, Vec2 delta) {
    drag_.onDrag(ctx, delta);
    resize_.onDrag(ctx, delta);
  }

  void NodeActor::onDragEnd(const EditorContext& ctx, Vec2) {
    drag_.commit(ctx, *location(), *selectionId());
    resize_.commit(ctx, *location(), *selectionId());
  }

  void NodeActor::onDragCancel() {
    drag_.cancel();
    resize_.cancel();
  }

}  // namespace fluir::editor
