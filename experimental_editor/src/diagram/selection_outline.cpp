#include "editor/diagram/selection_outline.hpp"

#include <QPainter>
#include <QPen>

#include "editor/scene/item_style.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::diagram {

  SelectionOutline::SelectionOutline(const EditorContext& ctx, QGraphicsItem& parent) :
    QGraphicsItem(&parent), ctx_(ctx) {
    setAcceptedMouseButtons(Qt::NoButton);
    setVisible(false);
  }

  void SelectionOutline::place(const Rect& frame) {
    prepareGeometryChange();
    const Rect out = draw::selectionOutline(frame, ctx_.layout);
    rect_ = {out.x, out.y, out.w, out.h};
  }

  void SelectionOutline::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    // Two concentric 1px rects, centred on pixel lines at selectionPad + 0.5 and + 1.5.
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(style::color(ctx_.theme.border), 1));
    const double inset = 0.5;
    painter->drawRect(rect_.adjusted(inset, inset, -inset, -inset));
    painter->drawRect(rect_.adjusted(1 + inset, 1 + inset, -1 - inset, -1 - inset));
  }

}  // namespace fluir::editor::diagram
