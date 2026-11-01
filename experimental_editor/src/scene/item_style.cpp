#include "editor/scene/item_style.hpp"

#include <cmath>
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>

#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::style {
  namespace {

    constexpr int FONT_PX = 13;
    constexpr int TAG_FONT_PX = 10;

  }  // namespace

  QColor color(Color c) { return QColor(c.r, c.g, c.b, c.a); }

  QRectF rect(const Rect& r) { return QRectF(r.x, r.y, r.w, r.h); }

  QRectF place(QGraphicsItem& item, const Rect& world) {
    const QPointF topLeft(world.x, world.y);
    item.setPos(item.parentItem() != nullptr ? item.parentItem()->mapFromScene(topLeft) : topLeft);
    return QRectF(0, 0, world.w, world.h);
  }

  QFont uiFont() {
    QFont font("DejaVu Sans Mono");
    font.setPixelSize(FONT_PX);
    return font;
  }

  QFont tagFont() {
    QFont font = uiFont();
    font.setPixelSize(TAG_FONT_PX);
    return font;
  }

  QFont font(FieldLook look) { return look == FieldLook::Tag ? tagFont() : uiFont(); }

  Qt::Alignment alignment(FieldLook look) {
    return look == FieldLook::Tag ? Qt::AlignLeft | Qt::AlignBottom : Qt::AlignLeft | Qt::AlignTop;
  }

  void shell(QPainter& painter, const QRectF& local, Color fill, const EditorContext& ctx) {
    painter.fillRect(local, color(fill));
    painter.setPen(QPen(color(ctx.theme.border), 0));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(local);
  }

  void tag(QPainter& painter, const QRectF& local, std::string_view tag, const EditorContext& ctx) {
    if (tag.empty()) {
      return;
    }
    painter.setFont(font(FieldLook::Tag));
    painter.setPen(color(ctx.theme.text));
    const double pad = ctx.layout.textPad;
    painter.drawText(local.adjusted(pad, pad, -pad, -pad),
                     alignment(FieldLook::Tag),
                     QString::fromUtf8(tag.data(), static_cast<qsizetype>(tag.size())));
  }

  void icon(QPainter& painter, const QString& resource, const QRectF& local, Color tint) {
    QSvgRenderer svg(resource);
    const QSizeF intrinsic = svg.defaultSize();
    const Rect fitted =
      fitInto(Rect{local.x(), local.y(), local.width(), local.height()}, Vec2{intrinsic.width(), intrinsic.height()});
    const QRectF target = rect(fitted);
    const QRectF device = painter.transform().mapRect(target);
    const QSize px(std::max(1, static_cast<int>(std::ceil(device.width()))),
                   std::max(1, static_cast<int>(std::ceil(device.height()))));
    if (target.isEmpty()) {
      return;
    }
    QImage image(px, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    {
      QPainter p(&image);
      svg.render(&p);
      p.setCompositionMode(QPainter::CompositionMode_SourceIn);
      p.fillRect(image.rect(), color(tint));
    }
    painter.drawImage(target, image);
  }

}  // namespace fluir::editor::style
