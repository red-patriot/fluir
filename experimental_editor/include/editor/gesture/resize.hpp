#ifndef FLUIR_EDITOR_GESTURE_RESIZE_HPP
#define FLUIR_EDITOR_GESTURE_RESIZE_HPP

#include "editor/gesture/gesture.hpp"

namespace fluir::editor {

  /** Which dimensions a resize drives. */
  enum class Axes { X, XY };

  /** Drags an actor's size across the grid, raising one ResizeTransaction on
   *  commit. Only the driven axes are clamped; the others are passed through. */
  class ResizeGesture : public Gesture {
   public:
    ResizeGesture(Axes axes, Limits<Vec2i> limits) : axes_(axes), limits_(limits) { }

    void update(const EditorContext& ctx, Vec2 worldDelta) override;
    FlowGraphLocation preview(const FlowGraphLocation& loc) const override;
    void commit(const EditorContext& ctx, const FlowGraphLocation& loc, const fluir::FullID& id) override;

    /** Whole grid units grown so far, not yet committed. */
    Vec2i delta() const { return delta_; }

   private:
    Axes axes_;
    Limits<Vec2i> limits_;
    GridAccumulator accumulator_;
    Vec2i delta_;
  };

}  // namespace fluir::editor

#endif
