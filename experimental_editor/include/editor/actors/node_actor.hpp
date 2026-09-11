#pragma once

#include <optional>
#include <utility>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
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
    using Actor::location;  // the const overload, hidden by the override below
    fluir::FlowGraphLocation* location() override = 0;

    /** This actor's node, as the parse tree would represent it. */
    virtual pt::Node node() const = 0;

    /** Resets every operand naming `nodeId`; returns the slots cleared (0 = lhs, 1 = rhs). */
    virtual std::vector<int> clearOperands(fluir::ID nodeId) { return {}; }

    /** Points operand `slot` back at `nodeId`. */
    virtual void restoreOperand(int slot, fluir::ID nodeId) { }

    void layout(const EditorContext& ctx) override;
    bool onDragStart(const EditorContext& ctx, Vec2 position) override;
    void onDrag(const EditorContext& ctx, Vec2 position, Vec2 delta) override;

   protected:
    DragHandle drag_;

   private:
    fluir::FullID id_;
  };

}  // namespace fluir::editor
