#include "editor/diagram/move_grip.hpp"

#include <QPainter>

#include "editor/scene/item_style.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::diagram {

  void MoveGrip::place(const Rect& frame) { setRect(moveGrip(frame, ctx_.layout.unitPx)); }

  void MoveGrip::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::icon(*painter, ":/images/dragHandle.svg", boundingRect(), ctx_.theme.border);
  }

  FlowGraphLocation MoveGrip::dragged(const FlowGraphLocation& start, Vec2i steps) const {
    FlowGraphLocation location = start;
    location.x += steps.x;
    location.y += steps.y;
    return location;
  }

}  // namespace fluir::editor::diagram
