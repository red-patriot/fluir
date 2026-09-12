#ifndef FLUIR_EDITOR_GESTURE_MOVE_HPP
#define FLUIR_EDITOR_GESTURE_MOVE_HPP

#include "editor/gesture/gesture.hpp"

namespace fluir::editor {

  /** Drags an actor across the grid, raising one MoveTransaction on commit. */
  class MoveGesture : public Gesture {
   public:
    void update(const EditorContext& ctx, Vec2 worldDelta) override;
    FlowGraphLocation preview(const FlowGraphLocation& loc) const override;
    void commit(const EditorContext& ctx, const FlowGraphLocation& loc, const fluir::FullID& id) override;

    /** Whole grid units moved so far, not yet committed. */
    Vec2i delta() const { return delta_; }

   private:
    GridAccumulator accumulator_;
    Vec2i delta_;
  };

}  // namespace fluir::editor

#endif
