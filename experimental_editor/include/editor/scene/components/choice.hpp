#ifndef FLUIR_EDITOR_SCENE_COMPONENTS_CHOICE_HPP
#define FLUIR_EDITOR_SCENE_COMPONENTS_CHOICE_HPP

#include <QGraphicsItem>
#include <QRectF>
#include <QString>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/field.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  class Choice : public QGraphicsItem {
   public:
    Choice(FullID path, Field field, QString text, const Rect& world, const EditorContext& ctx, QGraphicsItem* parent);

    const FullID& modelPath() const { return path_; }
    Field field() const { return field_; }
    const QString& text() const { return text_; }

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    FullID path_;
    Field field_;
    QRectF rect_;
    const EditorContext& ctx_;
    QString text_;
  };

}  // namespace fluir::editor

#endif
