#ifndef FLUIR_EDITOR_SCENE_COMPONENTS_TEXT_EDIT_HPP
#define FLUIR_EDITOR_SCENE_COMPONENTS_TEXT_EDIT_HPP

#include <QGraphicsTextItem>
#include <QRectF>
#include <QString>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/field.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  class TextEdit : public QGraphicsTextItem {
   public:
    TextEdit(
      FullID path, Field field, QString text, const Rect& world, const EditorContext& ctx, QGraphicsItem* parent);

    const FullID& modelPath() const { return path_; }
    Field field() const { return field_; }
    QString text() const { return toPlainText(); }

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    FullID path_;
    Field field_;
    QRectF rect_;
  };

}  // namespace fluir::editor

#endif
