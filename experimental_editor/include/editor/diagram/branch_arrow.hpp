#ifndef FLUIR_EDITOR_DIAGRAM_BRANCH_ARROW_HPP
#define FLUIR_EDITOR_DIAGRAM_BRANCH_ARROW_HPP

#include <QGraphicsItem>
#include <QRectF>

#include "editor/core/editor_context.hpp"

namespace fluir::editor::diagram {

  class Conditional;

  /** Shows its conditional's other branch on a press. Which branch shows is view state, so nothing is pushed. */
  class BranchArrow : public QGraphicsItem {
   public:
    BranchArrow(const EditorContext& ctx, Conditional& conditional);

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

   private:
    const EditorContext& ctx_;
    Conditional& conditional_;
    QRectF rect_;
  };

}  // namespace fluir::editor::diagram

#endif
