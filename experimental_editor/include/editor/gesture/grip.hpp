#ifndef FLUIR_EDITOR_GESTURE_GRIP_HPP
#define FLUIR_EDITOR_GESTURE_GRIP_HPP

#include <functional>
#include <memory>

#include "compiler/models/location.hpp"
#include "editor/core/geometry.hpp"
#include "editor/gesture/gesture.hpp"
#include "editor/gesture/resize.hpp"

namespace fluir::editor {

  class Actor;

  /** The parent-space rect a grip is measured inside. */
  using GripFrame = std::function<Rect(const Actor&, const EditorContext&)>;

  /** The actor's own rect: what almost every grip is placed against. */
  Rect actorBounds(const Actor& actor, const EditorContext& ctx);

  /** The band across the top of the actor, for frames that carry a header. */
  Rect actorHeader(const Actor& actor, const EditorContext& ctx);

  /** A spot an actor can be grabbed by, and the gesture grabbing it starts.
   *  An actor lists its grips in precedence order; the first hit wins. */
  struct Grip {
    /** The grip's rect in grid units, relative to `frame`. */
    std::function<Rect(const FlowGraphLocation&)> rect;
    /** Builds the gesture a press on this grip runs. */
    std::function<std::unique_ptr<Gesture>()> begin;
    GripFrame frame = actorBounds;
  };

  /** Grip rects in grid units, relative to their frame. */
  Rect dragGripRect(const FlowGraphLocation& loc);
  Rect horizResizeGripRect(const FlowGraphLocation& loc);
  Rect xyResizeGripRect(const FlowGraphLocation& loc);

  /** The grip an actor is moved by, inset from its frame's top-right corner. */
  Grip dragGrip(GripFrame frame = actorBounds);

  /** The bar down the right edge an actor's width is dragged by. */
  Grip horizResizeGrip(Limits<Vec2i> sizeUnits);

  /** The corner an actor's width and height are dragged by. */
  Grip xyResizeGrip(Limits<Vec2i> sizeUnits);

}  // namespace fluir::editor

#endif
