#pragma once

#include <optional>
#include <utility>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/actors/actor.hpp"
#include "editor/actors/port_actor.hpp"
#include "editor/components/drag_handle.hpp"

namespace fluir::editor {

  /** An Actor representing a node in the data-flow diagram. */
  class NodeActor : public PortActor {
   public:
    NodeActor(fluir::FullID id, Rect bounds) : PortActor(id.at(1), bounds), id_(std::move(id)) { }

    const fluir::FullID& id() const { return id_; }

    std::optional<fluir::FullID> selectionId() const override { return id_; }

    /** This node's body-local location (grid units); the DragHandle mutates it,
     *  and `layout` re-derives `bounds()` from it. */
    virtual const fluir::FlowGraphLocation& location() const = 0;

    void layout(const EditorContext& ctx) override;
    bool onDragStart(const EditorContext& ctx, Vec2 position) override;
    void onDrag(const EditorContext& ctx, Vec2 position, Vec2 delta) override;

   protected:
    /** `location()` names a plain member of the concrete node, so casting away
     *  its constness to let the drag move it is well-defined. */ // @CLAUDE let's discuss this rationale here
    fluir::FlowGraphLocation& mutableLocation() { return const_cast<fluir::FlowGraphLocation&>(location()); }

    DragHandle drag_;

   private:
    fluir::FullID id_;
  };

}  // namespace fluir::editor
