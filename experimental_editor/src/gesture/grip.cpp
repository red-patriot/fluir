#include "editor/gesture/grip.hpp"

#include "editor/actors/actor.hpp"
#include "editor/gesture/move.hpp"

namespace fluir::editor {
  namespace {
    constexpr double DRAG_SIZE = 3;
    constexpr double DRAG_INSET = 1;
    constexpr double HORIZ_RESIZE_WIDTH = 1;
    constexpr double XY_RESIZE_SIZE = 3;
  }  // namespace

  Rect actorBounds(const Actor& actor, const EditorContext&) { return actor.bounds(); }

  Rect actorHeader(const Actor& actor, const EditorContext& ctx) {
    const Rect& bounds = actor.bounds();
    return Rect{bounds.x, bounds.y, bounds.w, ctx.layout.headerH()};
  }

  Rect dragGripRect(const FlowGraphLocation& loc) {
    return Rect{.x = loc.width - (DRAG_SIZE + DRAG_INSET), .y = DRAG_INSET, .w = DRAG_SIZE, .h = DRAG_SIZE};
  }

  Rect horizResizeGripRect(const FlowGraphLocation& loc) {
    return Rect{
      .x = loc.width - HORIZ_RESIZE_WIDTH, .y = 0, .w = HORIZ_RESIZE_WIDTH, .h = static_cast<double>(loc.height)};
  }

  Rect xyResizeGripRect(const FlowGraphLocation& loc) {
    return Rect{
      .x = loc.width - XY_RESIZE_SIZE, .y = loc.height - XY_RESIZE_SIZE, .w = XY_RESIZE_SIZE, .h = XY_RESIZE_SIZE};
  }

  Grip dragGrip(GripFrame frame) {
    return Grip{
      .rect = dragGripRect, .begin = [] { return std::make_unique<MoveGesture>(); }, .frame = std::move(frame)};
  }

  Grip horizResizeGrip(Limits<Vec2i> sizeUnits) {
    return Grip{.rect = horizResizeGripRect,
                .begin = [sizeUnits] { return std::make_unique<ResizeGesture>(Axes::X, sizeUnits); },
                .frame = actorBounds};
  }

  Grip xyResizeGrip(Limits<Vec2i> sizeUnits) {
    return Grip{.rect = xyResizeGripRect,
                .begin = [sizeUnits] { return std::make_unique<ResizeGesture>(Axes::XY, sizeUnits); },
                .frame = actorBounds};
  }

}  // namespace fluir::editor
