#ifndef FLUIR_EDITOR_DIAGRAM_RESIZE_GRIP_HPP
#define FLUIR_EDITOR_DIAGRAM_RESIZE_GRIP_HPP

#include "editor/diagram/grip.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::diagram {

  /** Sizes its node along `axes`, within the node's size limits. */
  class ResizeGrip : public Grip {
   public:
    ResizeGrip(ResizeAxes axes, const EditorContext& ctx, FluirNode& node) : Grip(ctx, node), axes_(axes) { }

    ResizeAxes axes() const { return axes_; }

    /** Sits on `frame`'s right edge (X) or bottom-right corner (XY), in the node's coordinates. */
    void place(const Rect& frame);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    FlowGraphLocation dragged(const FlowGraphLocation& start, Vec2i steps) const override;

    ResizeAxes axes_;
  };

}  // namespace fluir::editor::diagram

#endif
