#ifndef FLUIR_EDITOR_SCENE_COMPONENTS_RESIZE_GRIP_HPP
#define FLUIR_EDITOR_SCENE_COMPONENTS_RESIZE_GRIP_HPP

#include <QGraphicsItem>
#include <QRectF>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/view/graph_layout.hpp"

namespace fluir::editor {

  class ResizeGrip : public QGraphicsItem {
   public:
    ResizeGrip(Part part, const Rect& world, const EditorContext& ctx, QGraphicsItem* parent);

    Part part() const { return part_; }

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    const EditorContext& ctx_;
    QRectF rect_;
    Part part_;
  };

}  // namespace fluir::editor

#endif
