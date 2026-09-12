#ifndef FLUIR_EDITOR_COMPONENTS_DRAG_HANDLE_HPP
#define FLUIR_EDITOR_COMPONENTS_DRAG_HANDLE_HPP

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {
  /** The grip's rect in grid units, inset from `nodeLoc`'s top-right corner. */
  Rect dragRect(const fluir::FlowGraphLocation& nodeLoc);

  /** The grip an actor is dragged by. Owns the uncommitted drag delta: the model
   *  is only written when the gesture commits. */
  class DragHandle {
   public:
    static constexpr double WIDTH = 3;
    static constexpr double HEIGHT = 3;

    /** True if `parentLocalPos` lands on the grip drawn inside `nodeRect`. */
    bool onDragStart(const EditorContext& ctx, Vec2 parentLocalPos, const FlowGraphLocation& loc, const Rect& nodeRect);

    /** Accumulates `worldDelta` into the preview, snapped to whole grid units. */
    void onDrag(const EditorContext& ctx, Vec2 worldDelta);

    /** Whole grid units this gesture has moved, not yet committed. */
    int dx() const { return dx_; }
    int dy() const { return dy_; }

    /** `loc` shifted by the live preview -- what layout and hit-testing must use. */
    FlowGraphLocation preview(const FlowGraphLocation& loc) const;

    /** Dispatches this gesture's edit, if it moved anything, and drops the preview. */
    void commit(const EditorContext& ctx, const FlowGraphLocation& loc, const fluir::FullID& id);

    /** Drops the preview without an edit. */
    void cancel();

    void draw(const Subview& view, const EditorContext& ctx, const FlowGraphLocation& loc, const Rect& nodeRect) const;

   private:
    Vec2 accumulator_{}; /**< sub-unit remainder of the live gesture */
    int dx_ = 0;         /**< whole grid units moved, not yet committed */
    int dy_ = 0;
    bool active_ = false;
  };
}  // namespace fluir::editor

#endif
