#pragma once

#include "editor/actors/actor.hpp"

namespace fluir::editor {

  /** Draws nothing itself: groups and clips a subtree. Popups, menus and
   *  toolbars are all ContainerActor subtrees. */
  class ContainerActor : public Actor {
   public:
    using Actor::Actor;

    // A container is scaffolding, so a miss on every child falls through to
    // whatever encloses it rather than stopping here.
    bool hittable() const override { return false; }
  };

}  // namespace fluir::editor
