#ifndef FLUIR_EDITOR_DIAGRAM_MOVE_GRIP_HPP
#define FLUIR_EDITOR_DIAGRAM_MOVE_GRIP_HPP

#include "editor/diagram/grip.hpp"

namespace fluir::editor::diagram {

  /** Moves its node. */
  class MoveGrip : public Grip {
   public:
    MoveGrip(const EditorContext& ctx, FluirNode& node) : Grip(ctx, node) { }

    /** Sits over `frame`'s top-right corner, in the node's coordinates. */
    void place(const Rect& frame);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    FlowGraphLocation dragged(const FlowGraphLocation& start, Vec2i steps) const override;
  };

}  // namespace fluir::editor::diagram

#endif
