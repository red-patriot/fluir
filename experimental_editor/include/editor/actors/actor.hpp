#pragma once

#include <vector>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  /** A node's port anchors, in body-local (pre-`toScreen`) coordinates. */
  struct PortSet {
    std::vector<Vec2> inputs;
    std::vector<Vec2> outputs;
  };

  class Actor {
   public:
    explicit Actor(Rect bounds) : bounds_(bounds) { }
    virtual ~Actor() = default;
    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;
    Actor(Actor&&) = delete;
    Actor& operator=(Actor&&) = delete;

    const Rect& bounds() const { return bounds_; }
    void setBounds(Rect bounds) { bounds_ = bounds; }

    virtual void onClick(Vec2 worldPos) = 0;

    /** Draw this actor's body/label/ports into `body`. */
    virtual void draw(const Subview& body, const EditorContext& ctx) const = 0;

   private:
    Rect bounds_;
  };

}  // namespace fluir::editor
