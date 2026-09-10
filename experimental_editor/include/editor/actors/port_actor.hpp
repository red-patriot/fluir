#pragma once

#include "compiler/models/id.hpp"
#include "editor/actors/actor.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** An actor a conduit can attach to: carries a body-scope id and port anchors. */
  class PortActor : public Actor {
   public:
    PortActor(fluir::ID portId, Rect bounds) : Actor(bounds), portId_(portId) { }

    fluir::ID portId() const { return portId_; }

    /** Anchors in body space, i.e. this actor's parent space. */
    virtual PortSet ports(const EditorContext& ctx) const = 0;

   private:
    fluir::ID portId_;
  };

}  // namespace fluir::editor
