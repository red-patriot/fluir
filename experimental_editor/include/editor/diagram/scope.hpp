#ifndef FLUIR_EDITOR_DIAGRAM_SCOPE_HPP
#define FLUIR_EDITOR_DIAGRAM_SCOPE_HPP

#include <QGraphicsItem>
#include <QGraphicsSceneContextMenuEvent>
#include <QRectF>

#include "compiler/models/id.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor::diagram {

  /** A region below a container's header band holding nodes and conduits. */
  class Scope : public QGraphicsItem {
   public:
    /** A scope over `local`, in its container's coordinates. */
    Scope(const Rect& local, QGraphicsItem* container);

    /** Covers `local`, in its container's coordinates. */
    void setRect(const Rect& local);

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   protected:
    /** Handles right click events. */
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

   private:
    QRectF rect_;
  };

  /** `scope`'s path in the exported tree. */
  fluir::FullID pathOf(const Scope& scope);

}  // namespace fluir::editor::diagram

#endif
