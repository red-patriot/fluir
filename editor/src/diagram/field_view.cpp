#include "editor/diagram/field_view.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <utility>

#include "editor/diagram/node.hpp"
#include "editor/scene/item_style.hpp"

namespace fluir::editor::diagram {

  FieldView::FieldView(
    FluirNode& node, FieldKey key, FieldLook look, Press press, const EditorContext& ctx, QGraphicsItem& parent) :
    QGraphicsItem(&parent),
    FieldPart(node, key),
    ctx_(ctx),
    target_(std::make_unique<FieldTarget>(node, std::move(key))),
    look_(look),
    press_(std::move(press)) {
    refresh();
  }

  void FieldView::refresh() {
    text_ = QString::fromStdString(target_->current());
    update();
  }

  QFont FieldView::font() const { return style::font(look_); }

  void FieldView::place(const Rect& rect) {
    prepareGeometryChange();
    rect_ = style::rect(rect);
  }

  void FieldView::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    const double pad = ctx_.layout.textPad;
    painter->setClipRect(rect_);
    painter->setFont(font());
    painter->setPen(style::color(ctx_.theme.text));
    painter->drawText(rect_.adjusted(pad, pad, -pad, -pad), style::alignment(look_), text_);
  }

  void FieldView::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
      event->ignore();
      return;
    }
    node().selectAlone();
    press_(*target_, event->screenPos());
  }

}  // namespace fluir::editor::diagram
