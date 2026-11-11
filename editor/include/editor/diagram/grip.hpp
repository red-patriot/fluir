#ifndef FLUIR_EDITOR_DIAGRAM_GRIP_HPP
#define FLUIR_EDITOR_DIAGRAM_GRIP_HPP

#include <optional>
#include <QGraphicsItem>
#include <QPointF>
#include <QRectF>

#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor::diagram {

  class FluirNode;

  /** A handle that drags its node's location in whole grid units. A press selects the node, a drag updates it live,
   *  Escape restores it, and release pushes one command. */
  class Grip : public QGraphicsItem {
   public:
    QRectF boundingRect() const override { return rect_; }

   protected:
    Grip(const EditorContext& ctx, FluirNode& node);

    /** Where a drag of `steps` grid units puts a node that started at `start`. */
    virtual FlowGraphLocation dragged(const FlowGraphLocation& start, Vec2i steps) const = 0;

    void setRect(const Rect& rect);

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

    const EditorContext& ctx_;
    FluirNode& node_;

   private:
    struct Gesture {
      FlowGraphLocation start;
      QPointF pressedAt;
    };

    QRectF rect_;
    std::optional<Gesture> gesture_;
  };

}  // namespace fluir::editor::diagram

#endif
