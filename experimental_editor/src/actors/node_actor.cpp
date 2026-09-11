#include "editor/actors/node_actor.hpp"

#include "editor/core/graph_geometry.hpp"

namespace fluir::editor {

  void NodeActor::layout(const EditorContext& ctx) {
    setBounds(localRect(*location(), ctx.layout.unitPx));
    Actor::layout(ctx);
  }

  bool NodeActor::onDragStart(const EditorContext& ctx, Vec2 position) {
    return drag_.onDragStart(ctx, position, *location(), bounds());
  }

  void NodeActor::onDrag(const EditorContext& ctx, Vec2, Vec2 delta) { drag_.onDrag(ctx, delta, *location()); }

}  // namespace fluir::editor
