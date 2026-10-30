#ifndef FLUIR_EDITOR_DIAGRAM_SCOPE_HPP
#define FLUIR_EDITOR_DIAGRAM_SCOPE_HPP

#include <QGraphicsItem>
#include <QRectF>

#include "editor/core/geometry.hpp"

namespace fluir::editor::diagram {

  /** A region below a container's header band holding nodes and conduits. */
  class Scope : public QGraphicsItem {
   public:
    /** A scope over `local`, in its container's coordinates. */
    Scope(const Rect& local, QGraphicsItem* container);

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    QRectF rect_;
  };

}  // namespace fluir::editor::diagram

#endif
