#ifndef FLUIR_EDITOR_SCENE_COMPONENTS_FRAME_HPP
#define FLUIR_EDITOR_SCENE_COMPONENTS_FRAME_HPP

#include <QGraphicsItem>
#include <QRectF>
#include <QString>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  class Frame : public QGraphicsItem {
   public:
    Frame(const Rect& world, Color header, QString tag, const EditorContext& ctx, QGraphicsItem* parent);

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    const EditorContext& ctx_;
    QRectF rect_;
    Color header_;
    QString tag_;
  };

}  // namespace fluir::editor

#endif
