#pragma once

#include <concepts>
#include <memory>
#include <utility>
#include <vector>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  /** A node's port anchors, in body-local coordinates. */
  struct PortSet {
    std::vector<Vec2> inputs;
    std::vector<Vec2> outputs;
  };

  /** A node in the display tree. */
  class Actor {
   public:
    /** Whether children are offset+clipped to `bounds()`. Scene roots opt out. */
    enum class ClipChildren { Yes, No };  // @CLAUDE explain why this is necessary?

    explicit Actor(Rect bounds, ClipChildren clip = ClipChildren::Yes) : bounds_(bounds), clip_(clip) { }
    virtual ~Actor() = default;
    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;
    Actor(Actor&&) = delete;
    Actor& operator=(Actor&&) = delete;

    const Rect& bounds() const { return bounds_; }
    void setBounds(Rect bounds) { bounds_ = bounds; }

    /** Takes ownership of `child`. */
    template <typename ActorType>
      requires std::derived_from<ActorType, Actor>
    ActorType& add(std::unique_ptr<ActorType> child) {
      child->parent_ = this;
      children_.push_back(std::move(child));
      return static_cast<ActorType&>(*children_.back());
    }

    void clearChildren() { children_.clear(); }

    const std::vector<std::unique_ptr<Actor>>& children() const { return children_; }
    Actor* parent() const { return parent_; }

    /** This actor's rect in world space, accumulated through its ancestors. */
    Rect worldBounds() const {
      const Vec2 origin = parentOrigin() + bounds_.topLeft();
      return {origin.x, origin.y, bounds_.w, bounds_.h};
    }

    /** World origin of this actor's parent space. */
    Vec2 parentOrigin() const {
      Vec2 origin;
      for (const Actor* p = parent_; p != nullptr; p = p->parent_) {
        origin = origin + p->bounds_.topLeft();
      }
      return origin;
    }

    /** Converts a world position into this actor's parent space. */
    Vec2 toParentLocal(Vec2 world) const { return world - parentOrigin(); }

    /** Deepest, last-painted descendant containing `parentLocalPos`, else this
     *  actor if it is hittable, else nullptr. */
    Actor* hitTest(Vec2 parentLocalPos) {
      if (clip_ == ClipChildren::Yes && !bounds_.contains(parentLocalPos)) {
        return nullptr;
      }
      const Vec2 local = parentLocalPos - bounds_.topLeft();
      for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if (Actor* hit = (*it)->hitTest(local)) {
          return hit;
        }
      }
      return hittable() ? this : nullptr;
    }

    /** Whether a hit that lands on no child resolves to this actor. */
    virtual bool hittable() const { return true; }

    /** Re-derives this actor's `bounds()` from its model state, then its
     *  children's. Runs before hit-testing and drawing, never during either. */
    virtual void layout(const EditorContext& ctx) {
      for (const auto& child : children_) {
        child->layout(ctx);
      }
    }

    /** Handle a click event at `position`, in parent space. */
    virtual void onClick(Vec2 position) { }

    /** Optionally start dragging a node.
     * Returning true claims the drag gesture, false leaves unclaimed. */
    virtual bool onDragStart(const EditorContext& ctx, Vec2 position) { return false; }

    /** Callback invoked each frame while dragging.
     * `delta` is the displacement since the last call, in world pixels. */
    virtual void onDrag(const EditorContext& ctx, Vec2 position, Vec2 delta) { }

    /** Called once on MouseUp, ending a claimed drag. */
    virtual void onDragEnd(const EditorContext& ctx, Vec2 position) { }

    /** Draws this actor into its parent's view, then its children into a nested
     *  view anchored (and clipped) to `bounds()`. */
    void draw(const Subview& parentView, const EditorContext& ctx) const {
      drawSelf(parentView, ctx);
      if (children_.empty()) {
        return;
      }
      if (clip_ == ClipChildren::No) {
        drawChildren(parentView, ctx);
        return;
      }
      const Subview view = parentView.child(bounds_);
      drawChildren(view, ctx);
    }

   protected:
    /** Draws this actor's own chrome, in the *parent's* view, so a frame's
     *  border is not clipped away by its own subview. */
    virtual void drawSelf(const Subview& parentView, const EditorContext& ctx) const { }

   private:
    void drawChildren(const Subview& view, const EditorContext& ctx) const {
      for (const auto& child : children_) {
        child->draw(view, ctx);
      }
    }

    Rect bounds_;
    ClipChildren clip_;
    Actor* parent_ = nullptr;
    std::vector<std::unique_ptr<Actor>> children_;
  };

}  // namespace fluir::editor
