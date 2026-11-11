#ifndef FLUIR_EDITOR_DIAGRAM_CONDUIT_HPP
#define FLUIR_EDITOR_DIAGRAM_CONDUIT_HPP

#include <QGraphicsPathItem>
#include <QPointF>
#include <vector>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/diagram/terminal.hpp"

namespace fluir::editor::diagram {

  /** Display element for a conduit. */
  class Conduit : public QGraphicsPathItem {
   public:
    /** A conduit attached to its terminals. */
    Conduit(
      fluir::ID id, Terminal* source, std::vector<Terminal*> sinks, const EditorContext& ctx, QGraphicsItem* scope);

    fluir::ID id() const { return id_; }
    Terminal* source() const { return source_; }
    const std::vector<Terminal*>& sinks() const { return sinks_; }

    /** Replaces the sinks, attaching to the new ones. */
    void setSinks(std::vector<Terminal*> sinks);

    /** Starts following its terminals, and re-routes. */
    void attach();
    /** Stops following its terminals, e.g. while it is out of the scene. */
    void detach();

    /** Redraws a line from the source to each sink, wherever they now are. */
    void route();

    /** Only a band around its lines, not the area they enclose. */
    QPainterPath shape() const override;
    QRectF boundingRect() const override;
    bool collidesWithPath(const QPainterPath& path, Qt::ItemSelectionMode mode) const override;

   protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

   private:
    /** Whether `scenePos` is on one of its end terminals. */
    bool endsAt(QPointF scenePos) const;
    fluir::ID id_;
    Terminal* source_;
    std::vector<Terminal*> sinks_;
    double hitPx_;
  };

}  // namespace fluir::editor::diagram

#endif
