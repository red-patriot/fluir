#include "editor/scene/components/choice.hpp"

#include <QPainter>
#include <utility>

#include "editor/scene/item_style.hpp"

namespace fluir::editor {

  Choice::Choice(
    FullID path, Field field, QString text, const Rect& world, const EditorContext& ctx, QGraphicsItem* parent) :
    QGraphicsItem(parent),
    path_(std::move(path)),
    field_(field),
    rect_(style::place(*this, world)),
    ctx_(ctx),
    text_(std::move(text)) { }

  void Choice::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    const double pad = ctx_.layout.textPad;
    painter->setClipRect(rect_);
    painter->setFont(style::uiFont());
    painter->setPen(style::color(ctx_.theme.text));
    painter->drawText(rect_.adjusted(pad, pad, -pad, -pad), Qt::AlignLeft | Qt::AlignTop, text_);
  }

}  // namespace fluir::editor
