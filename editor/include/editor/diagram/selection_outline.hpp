#ifndef FLUIR_EDITOR_DIAGRAM_SELECTION_OUTLINE_HPP
#define FLUIR_EDITOR_DIAGRAM_SELECTION_OUTLINE_HPP

#include <QGraphicsItem>
#include <QRectF>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor::diagram {

  /** A frame drawn just outside a node while it is selected. Takes no mouse input. */
  class SelectionOutline : public QGraphicsItem {
   public:
    explicit SelectionOutline(const EditorContext& ctx, QGraphicsItem& parent);

    /** Surrounds `frame`, in the node's coordinates. */
    void place(const Rect& frame);

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    const EditorContext& ctx_;
    QRectF rect_;
  };

}  // namespace fluir::editor::diagram

#endif
