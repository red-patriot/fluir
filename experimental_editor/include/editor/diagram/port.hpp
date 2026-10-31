#ifndef FLUIR_EDITOR_DIAGRAM_PORT_HPP
#define FLUIR_EDITOR_DIAGRAM_PORT_HPP

#include <optional>
#include <QGraphicsItem>
#include <QPointF>
#include <QRectF>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/diagram/terminal.hpp"
#include "editor/view/draw/conditional.hpp"

namespace fluir::editor::diagram {

  class Conditional;

  /** A port that carries data from outside a sub-block to inside it. A drag slides it along its wall in whole grid
   *  units, Escape restores it, and release pushes one command. */
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

    /** Moves to `placed`'s top-left, in the conditional's coordinates. */
    void place(const Rect& placed) { setPos(placed.x, placed.y); }

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

   private:
    struct Gesture {
      int start;
      QPointF pressedAt;
    };

    Conditional& conditional() const;
    int y() const;

    const EditorContext& ctx_;
    QRectF rect_;
    draw::PortRef ref_;
    fluir::ID innerId_;
    Terminal* outer_;
    Terminal* inner_ = nullptr;
    std::optional<Gesture> gesture_;
  };

}  // namespace fluir::editor::diagram

#endif
