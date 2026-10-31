#include "editor/diagram/field_text.hpp"

#include <QKeyEvent>
#include <QPainter>
#include <QTextCursor>
#include <QTextDocument>
#include <string>

#include "editor/commands/set_field.hpp"
#include "editor/diagram/node.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor::diagram {

  FieldText::FieldText(FluirNode& node, Field field, const EditorContext& ctx, QGraphicsItem& parent) :
    QGraphicsTextItem(&parent), FieldPart(node, field), ctx_(ctx) {
    setTextInteractionFlags(Qt::TextEditorInteraction);
    setDefaultTextColor(style::color(ctx.theme.text));
    setFont(style::uiFont());
    refresh();
    connect(document(), &QTextDocument::contentsChanged, this, [this] {
      valid_ = this->node().accepts(this->field(), toPlainText().toStdString());
      update();
    });
  }

  void FieldText::refresh() { setPlainText(QString::fromStdString(node().text(field()))); }

  void FieldText::place(const Rect& rect) {
    setPos(rect.x, rect.y);
    if (multiline()) {
      setTextWidth(rect.w);
    }
  }

  void FieldText::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    QGraphicsTextItem::paint(painter, option, widget);
    if (!valid_) {
      painter->setPen(QPen(style::color(ctx_.theme.error), 0));
      painter->setBrush(Qt::NoBrush);
      painter->drawRect(boundingRect());
    }
  }

  void FieldText::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    node().selectAlone();
    QGraphicsTextItem::mousePressEvent(event);
  }

  void FieldText::keyPressEvent(QKeyEvent* event) {
    const bool enter = event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter;
    if (enter && !(multiline() && event->modifiers() & Qt::ShiftModifier)) {
      if (valid_) {
        commit();
        clearFocus();
      }
    } else if (event->key() == Qt::Key_Escape) {
      refresh();
      clearFocus();
    } else {
      QGraphicsTextItem::keyPressEvent(event);
    }
  }

  void FieldText::focusOutEvent(QFocusEvent* event) {
    commit();
    QTextCursor cursor = textCursor();
    cursor.clearSelection();
    setTextCursor(cursor);
    QGraphicsTextItem::focusOutEvent(event);
  }

  void FieldText::commit() {
    const std::string draft = toPlainText().toStdString();
    const std::string current = node().text(field());
    if (valid_ && draft != current) {
      ModuleScene::of(*this).undoStack().push(new SetField(node(), field(), current, draft));
    } else if (draft != current) {
      refresh();
    }
  }

}  // namespace fluir::editor::diagram
