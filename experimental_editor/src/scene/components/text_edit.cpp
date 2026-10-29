#include "editor/scene/components/text_edit.hpp"

#include <QPainter>
#include <QTextDocument>
#include <utility>

#include "editor/scene/item_style.hpp"

namespace fluir::editor {

  TextEdit::TextEdit(
    FullID path, Field field, QString text, const Rect& world, const EditorContext& ctx, QGraphicsItem* parent) :
    QGraphicsTextItem(std::move(text), parent),
    path_(std::move(path)),
    field_(field),
    rect_(style::place(*this, world)) {
    setFont(style::uiFont());
    setDefaultTextColor(style::color(ctx.theme.text));
    document()->setDocumentMargin(ctx.layout.textPad);
    // Only a comment's text wraps; every other field is one line, clipped to its label.
    if (field.kind == Field::Kind::Text) {
      setTextWidth(world.w);
    }
  }

  void TextEdit::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    painter->setClipRect(rect_);
    QGraphicsTextItem::paint(painter, option, widget);
  }

}  // namespace fluir::editor
