#ifndef FLUIR_EDITOR_COMPONENTS_DRAG_HANDLE_HPP
#define FLUIR_EDITOR_COMPONENTS_DRAG_HANDLE_HPP

#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {
  /** The grip's rect in grid units, inset from `nodeLoc`'s top-right corner. */
  Rect dragRect(const fluir::FlowGraphLocation& nodeLoc);

  /** The grip an actor is dragged by. Owns only the sub-grid remainder */
  class DragHandle {
   public:
    static constexpr double WIDTH = 3;
    static constexpr double HEIGHT = 3;

    /** True if `parentLocalPos` lands on the grip drawn inside `nodeRect`. */
    bool onDragStart(const EditorContext& ctx, Vec2 parentLocalPos, const FlowGraphLocation& loc, const Rect& nodeRect);

    /** Applies `worldDelta` to `loc`, snapped to whole grid units. */
    void onDrag(const EditorContext& ctx, Vec2 worldDelta, FlowGraphLocation& loc);

    void draw(const Subview& view, const EditorContext& ctx, const FlowGraphLocation& loc, const Rect& nodeRect) const;

   private:
    Vec2 accumulator_{}; /**< Accumulated diff while dragging */
  };
}  // namespace fluir::editor

#endif
