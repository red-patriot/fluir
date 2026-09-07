#pragma once

#include <utility>

#include "compiler/models/id.hpp"
#include "editor/actors/actor.hpp"

namespace fluir::editor {

  /** An Actor representing a node in the data-flow diagram. */
  class NodeActor : public Actor {
   public:
    NodeActor(fluir::FullID id, Rect bounds) : Actor(bounds), id_(std::move(id)) { }

    const fluir::FullID& id() const { return id_; }

    /** This node's port anchors, in body-local (pre-`toScreen`) coordinates. */
    virtual PortSet ports(const EditorContext& ctx) const = 0;

    /** Claim the drag gesture only when `worldPos` lands on the drag handle. */
    bool onDragStart(const EditorContext& ctx, Vec2 worldPos) override;
    /** Accumulate sub-grid pixels and nudge the node by whole grid units. */
    void onDrag(const EditorContext& ctx, Vec2 worldPos, Vec2 worldDelta) override;
    // onDragEnd: inherited no-op.

    /** Solid border-colored block in the node's top-left corner. Concrete
     *  draw() calls this last, passing the node rect it already computed. */
    void drawDragHandle(const Subview& body, const EditorContext& ctx, Rect nodeRect) const;

   protected:
    /** Shift this node's grid location by whole units. */
    virtual void nudgeLocation(int dxUnits, int dyUnits) = 0;

   private:
    Rect dragHandleWorldRect(const EditorContext& ctx) const;

    fluir::FullID id_;
    Vec2 dragAccum_{};  // sub-grid remainder in world px, reset on each onDragStart
  };

}  // namespace fluir::editor
