#ifndef FLUIR_EDITOR_GESTURE_GESTURE_HPP
#define FLUIR_EDITOR_GESTURE_GESTURE_HPP

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** An in-progress pointer gesture. It exists only while it runs, so there is
   *  no idle state to represent and destroying it is the cancel. */
  class Gesture {
   public:
    virtual ~Gesture() = default;

    /** Folds `worldDelta` into the live preview. */
    virtual void update(const EditorContext& ctx, Vec2 worldDelta) = 0;

    /** `loc` with this gesture applied -- what layout, drawing and hit-testing use. */
    virtual FlowGraphLocation preview(const FlowGraphLocation& loc) const = 0;

    /** Raises this gesture's edit, if it changed anything. */
    virtual void commit(const EditorContext& ctx, const FlowGraphLocation& loc, const fluir::FullID& id) = 0;
  };

  /** Folds sub-unit world deltas into whole grid-unit steps, carrying the remainder. */
  class GridAccumulator {
   public:
    /** Whole grid units `worldDelta` crosses; the sub-unit remainder is kept. */
    Vec2i fold(double unitPx, Vec2 worldDelta);

   private:
    Vec2 remainder_;
  };

}  // namespace fluir::editor

#endif
