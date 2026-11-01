#include "editor/diagram/field_text.hpp"

#include <memory>
#include <QKeyEvent>
#include <QPainter>
#include <QTextCursor>
#include <QTextDocument>
#include <string>
#include <utility>

#include "editor/diagram/node.hpp"
#include "editor/scene/item_style.hpp"

namespace fluir::editor::diagram {

  FieldText::FieldText(FluirNode& node, FieldKey key, FieldLook look, const EditorContext& ctx, QGraphicsItem& parent) :
    QGraphicsTextItem(&parent),
    FieldPart(node, key),
    ctx_(ctx),
    target_(std::make_unique<FieldTarget>(node, std::move(key))) {
    setTextInteractionFlags(Qt::TextEditorInteraction);
    setDefaultTextColor(style::color(ctx.theme.text));
    setFont(style::font(look));
    refresh();
    connect(document(), &QTextDocument::contentsChanged, this, [this] {
      valid_ = target_->validate(toPlainText().toStdString());
      update();
    });
  }

  void FieldText::refresh() { setPlainText(QString::fromStdString(target_->current())); }

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
    const std::string current = target_->current();
    if (valid_ && draft != current) {
      target_->commit(draft);
    } else if (draft != current) {
      refresh();
    }
  }

}  // namespace fluir::editor::diagram
