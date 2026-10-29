#ifndef FLUIR_EDITOR_SCENE_COMPONENTS_BRANCH_ARROW_HPP
#define FLUIR_EDITOR_SCENE_COMPONENTS_BRANCH_ARROW_HPP

#include <QGraphicsItem>
#include <QRectF>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  class BranchArrow : public QGraphicsItem {
   public:
    BranchArrow(const Rect& world, const EditorContext& ctx, QGraphicsItem* parent);

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    const EditorContext& ctx_;
    QRectF rect_;
  };

}  // namespace fluir::editor

#endif
