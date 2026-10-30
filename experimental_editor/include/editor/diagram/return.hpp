#ifndef FLUIR_EDITOR_DIAGRAM_RETURN_HPP
#define FLUIR_EDITOR_DIAGRAM_RETURN_HPP

#include <QGraphicsItem>
#include <QRectF>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/view/draw/function.hpp"

namespace fluir::editor::diagram {

  /** A function return. */
  class Return : public QGraphicsItem {
   public:
    Return(const draw::Signature& signature,
           fluir::ID id,
           const Rect& frame,
           const EditorContext& ctx,
           QGraphicsItem* function);

    fluir::ID id() const { return id_; }

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    const EditorContext& ctx_;
    Color color_;
    fluir::ID id_;
    QRectF rect_;
  };

}  // namespace fluir::editor::diagram

#endif
