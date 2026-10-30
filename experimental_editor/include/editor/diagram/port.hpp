#ifndef FLUIR_EDITOR_DIAGRAM_PORT_HPP
#define FLUIR_EDITOR_DIAGRAM_PORT_HPP

#include <QGraphicsItem>
#include <QRectF>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/diagram/terminal.hpp"
#include "editor/view/draw/conditional.hpp"

namespace fluir::editor::diagram {

  /** A port that carries data from outside a sub-block to inside it */
  class Port : public QGraphicsItem {
   public:
    Port(const draw::Ports& ports,
         draw::PortRef ref,
         const Rect& frame,
         const EditorContext& ctx,
         QGraphicsItem* conditional);

    draw::PortRef ref() const { return ref_; }
    fluir::ID innerId() const { return innerId_; }
    /** The outer scope's terminal. */
    Terminal* outer() const { return outer_; }
    /** The terminal the inner scope wires to. */
    Terminal* inner() const { return inner_; }

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    const EditorContext& ctx_;
    QRectF rect_;
    draw::PortRef ref_;
    fluir::ID innerId_;
    Terminal* outer_;
    Terminal* inner_ = nullptr;
  };

}  // namespace fluir::editor::diagram

#endif
