#ifndef FLUIR_EDITOR_DIAGRAM_PARAMETER_HPP
#define FLUIR_EDITOR_DIAGRAM_PARAMETER_HPP

#include <QGraphicsItem>
#include <QRectF>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/view/draw/function.hpp"

namespace fluir::editor::diagram {

  /** A function parameter. */
  class Parameter : public QGraphicsItem {
   public:
    /** `id`'s entry of `signature`, placed by its function's `frame` (in the function's coordinates). */
    Parameter(const draw::Signature& signature,
              fluir::ID id,
              const Rect& frame,
              const EditorContext& ctx,
              QGraphicsItem* function);

    fluir::ID id() const { return id_; }

    /** Moves to `placed`'s top-left, in the function's coordinates. */
    void place(const Rect& placed) { setPos(placed.x, placed.y); }

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

   private:
    const EditorContext& ctx_;
    Color color_;
    fluir::ID id_;
    QRectF rect_;
  };

}  // namespace fluir::editor::diagram

#endif
