#include "editor/diagram/branch_arrow.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "editor/diagram/conditional.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/view/draw/conditional.hpp"

namespace fluir::editor::diagram {

  // The arrow sits by the header's tag, which never moves with the frame's size.
  BranchArrow::BranchArrow(const EditorContext& ctx, Conditional& conditional) :
    QGraphicsItem(&conditional),
    ctx_(ctx),
    conditional_(conditional),
    rect_(style::rect(draw::branchArrowRect({0, 0, 0, ctx.layout.headerH()}, ctx.layout))) { }

  void BranchArrow::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::icon(*painter, ":/images/branchArrow.svg", rect_, ctx_.theme.text);
  }

  void BranchArrow::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
      event->ignore();
      return;
    }
    conditional_.selectAlone();
    conditional_.switchBranch();
  }

}  // namespace fluir::editor::diagram
