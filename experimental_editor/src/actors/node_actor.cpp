#include "editor/actors/node_actor.hpp"

#include "editor/core/graph_geometry.hpp"

namespace fluir::editor {
  namespace {
    // TODO: Make configurable. A node's height follows its content, so it is unbounded.
    constexpr Limits<Vec2i> SIZE_UNITS{.lower = Vec2i{4, 0}, .upper = Vec2i{1000, 1000}};
  }  // namespace

  // The drag grip is listed first, so it keeps every pixel it shares with the bar.
  NodeActor::NodeActor(fluir::FullID id, Rect bounds) :
    PortActor(id.at(1), bounds),
    gestures_(*this, {dragGrip(), horizResizeGrip(SIZE_UNITS)}, SIZE_UNITS),
    id_(std::move(id)) { }

  void NodeActor::layout(const EditorContext& ctx) {
    setBounds(localRect(previewLocation(), ctx.layout.unitPx));
    Actor::layout(ctx);
  }

}  // namespace fluir::editor
