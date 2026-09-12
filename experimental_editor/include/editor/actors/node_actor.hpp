#pragma once

#include <optional>
#include <utility>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/actors/actor.hpp"
#include "editor/actors/port_actor.hpp"
#include "editor/gesture/gesture_host.hpp"
#include "editor/gesture/grip.hpp"

namespace fluir::editor {

  /** An Actor representing a node in the data-flow diagram. */
  class NodeActor : public PortActor {
   public:
    NodeActor(fluir::FullID id, Rect bounds);

    const fluir::FullID& id() const { return id_; }

    std::optional<fluir::FullID> selectionId() const override { return id_; }

    /** This node's body-local location (grid units); `layout` re-derives
     *  `bounds()` from it, shifted by any uncommitted drag preview. */
    using Actor::location;  // the const overload, hidden by the override below
    fluir::FlowGraphLocation* location() override = 0;

    /** This actor's node, as the parse tree would represent it. */
    virtual pt::Node node() const = 0;

    /** Resets every operand naming `nodeId`; returns the slots cleared (0 = lhs, 1 = rhs). */
    virtual std::vector<int> clearOperands(fluir::ID nodeId) { return {}; }

    /** Points operand `slot` back at `nodeId`. */
    virtual void restoreOperand(int slot, fluir::ID nodeId) { }

    /** This node's location with any live gesture applied. */
    fluir::FlowGraphLocation previewLocation() const { return gestures_.preview(*location()); }

    void layout(const EditorContext& ctx) override;
    GestureHost* gestures() override { return &gestures_; }

   protected:
    GestureHost gestures_;

   private:
    fluir::FullID id_;
  };

}  // namespace fluir::editor
