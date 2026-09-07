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

   private:
    fluir::FullID id_;
  };

}  // namespace fluir::editor
