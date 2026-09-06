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

    /** Handle a click event at `position`. */
    virtual void onClick(Vec2 position) = 0;

    /** Optionally start dragging a node.
     * Returning true claims the drag gesture, false leaves unclaimed. */
    virtual bool onDragStart(Vec2 position) { return false; }

    /** Callback invoked each frame while dragging.
     * `delta` is the displacement since the last call. */
    virtual void onDrag(Vec2 position, Vec2 delta) { }

    /** Called once on MouseUp, ending a claimed drag. */
    virtual void onDragEnd(Vec2) { }

    /** Draw this actor into `view`. */
    virtual void draw(const Subview& view, const EditorContext& ctx) const = 0;

   private:
    Rect bounds_;
  };

}  // namespace fluir::editor
