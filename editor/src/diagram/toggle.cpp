#include "editor/diagram/toggle.hpp"

#include <memory>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <utility>

#include "editor/diagram/edits.hpp"
#include "editor/diagram/node.hpp"
#include "editor/scene/item_style.hpp"

namespace fluir::editor::diagram {

  Toggle::Toggle(FluirNode& node, FieldKey key, const EditorContext& ctx, QGraphicsItem& parent) :
    QGraphicsItem(&parent),
    FieldPart(node, key),
    ctx_(ctx),
    target_(std::make_unique<FieldTarget>(node, std::move(key))) {
    refresh();
  }

  void Toggle::refresh() {
    value_ = target_->current() == "true";
    update();
  }

  void Toggle::place(const Rect& rect) {
    prepareGeometryChange();
    rect_ = style::rect(rect);
  }

  void Toggle::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::icon(*painter, value_ ? ":/images/true.svg" : ":/images/false.svg", rect_, ctx_.theme.border);
  }

  void Toggle::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
      event->ignore();
      return;
    }
    node().selectAlone();
    flip(*target_);
  }

}  // namespace fluir::editor::diagram
