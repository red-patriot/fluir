#ifndef FLUIR_EDITOR_SCENE_ITEM_STYLE_HPP
#define FLUIR_EDITOR_SCENE_ITEM_STYLE_HPP

#include <QColor>
#include <QFont>
#include <QGraphicsItem>
#include <QPainter>
#include <QRectF>
#include <QString>
#include <string_view>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::style {

  QColor color(Color c);
  QRectF rect(const Rect& r);

  /** Moves `item` under its parent so it covers `world` (scene px); returns its local rect. */
  QRectF place(QGraphicsItem& item, const Rect& world);

  /** The UI font. */
  QFont uiFont();
  /** The font of a label tag. */
  QFont tagFont();
  /** The font a field shows its value in. */
  QFont font(FieldLook look);
  /** Where in its rect a field's value sits. */
  Qt::Alignment alignment(FieldLook look);

  /** Fills `local` with `fill` and draws its border. */
  void shell(QPainter& painter, const QRectF& local, Color fill, const EditorContext& ctx);
  /** Draws `tag` small along the bottom-left of `local`. */
  void tag(QPainter& painter, const QRectF& local, std::string_view tag, const EditorContext& ctx);
  /** The SVG at `resource`, fitted into `local` keeping its aspect, recolored to `tint`. */
  void icon(QPainter& painter, const QString& resource, const QRectF& local, Color tint);

}  // namespace fluir::editor::style

#endif
