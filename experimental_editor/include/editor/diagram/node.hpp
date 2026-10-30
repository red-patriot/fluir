#ifndef FLUIR_EDITOR_DIAGRAM_NODE_HPP
#define FLUIR_EDITOR_DIAGRAM_NODE_HPP

#include <QGraphicsItem>
#include <QRectF>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor::diagram {

  /** Display element for a node in a Fluir data-flow diagram. */
  class FluirNode : public QGraphicsItem {
   public:
    virtual fluir::ID id() const = 0;
    virtual const FlowGraphLocation& location() const = 0;

    QRectF boundingRect() const override;

   protected:
    FluirNode(const EditorContext& ctx, QGraphicsItem* parent);

    /** Moves this node to its location within its scope. Call once the location is set. */
    void place();
    /** This node's rect in its own coordinates. */
    Rect frame() const;

    const EditorContext& ctx_;
  };

}  // namespace fluir::editor::diagram

#endif
