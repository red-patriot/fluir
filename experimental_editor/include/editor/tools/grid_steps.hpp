#ifndef FLUIR_EDITOR_TOOLS_GRID_STEPS_HPP
#define FLUIR_EDITOR_TOOLS_GRID_STEPS_HPP

#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** Turns world motion into whole grid steps. */
  class GridSteps {
   public:
    void start(Vec2 world);
    /** The whole steps moved since the last call. Keeps the remainder for the next step(s). */
    Vec2i advance(Vec2 world, double unit);

   private:
    Vec2 last_;
    Vec2 remainder_;
  };

}  // namespace fluir::editor

#endif
