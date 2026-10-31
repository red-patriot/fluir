#include "editor/diagram/toggle.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "editor/commands/set_field.hpp"
#include "editor/diagram/node.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor::diagram {
  namespace {

    const char* spelled(bool value) { return value ? "true" : "false"; }

  }  // namespace

  Toggle::Toggle(FluirNode& node, Field field, const EditorContext& ctx, QGraphicsItem& parent) :
    QGraphicsItem(&parent), FieldPart(node, field), ctx_(ctx) {
    refresh();
  }

  void Toggle::refresh() {
    value_ = node().text(field()) == spelled(true);
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
    ModuleScene::of(*this).undoStack().push(new SetField(node(), field(), spelled(value_), spelled(!value_)));
  }

}  // namespace fluir::editor::diagram
