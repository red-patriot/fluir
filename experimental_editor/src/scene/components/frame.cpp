#include "editor/scene/components/frame.hpp"

#include <QPainter>
#include <utility>

#include "editor/scene/item_style.hpp"

namespace fluir::editor {

  Frame::Frame(const Rect& world, Color header, QString tag, const EditorContext& ctx, QGraphicsItem* parent) :
    QGraphicsItem(parent), ctx_(ctx), rect_(style::place(*this, world)), header_(header), tag_(std::move(tag)) { }

  void Frame::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    const QRectF band(rect_.x(), rect_.y(), rect_.width(), ctx_.layout.headerH());
    painter->fillRect(band, style::color(header_));
    painter->setPen(QPen(style::color(ctx_.theme.border), 0));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect_);
    style::tag(*painter, band, tag_.toStdString(), ctx_);
  }

}  // namespace fluir::editor
