#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  class GestureHost;
  class InlineEdit;

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

    /** Removes `child`, returning ownership. Null when it is not a child of this. */
    std::unique_ptr<Actor> detach(const Actor& child) {
      const std::size_t index = indexOf(child);
      if (index == children_.size()) {
        return nullptr;
      }
      std::unique_ptr<Actor> out = std::move(children_[index]);
      children_.erase(children_.begin() + static_cast<std::ptrdiff_t>(index));
      out->parent_ = nullptr;
      return out;
    }

    /** Re-inserts at `index`, restoring draw order. */
    Actor& insert(std::size_t index, std::unique_ptr<Actor> child) {
      child->parent_ = this;
      const std::size_t at = std::min(index, children_.size());
      return **children_.insert(children_.begin() + static_cast<std::ptrdiff_t>(at), std::move(child));
    }

    /** `child`'s draw-order position, or `children().size()` when it is not a child. */
    std::size_t indexOf(const Actor& child) const {
      for (std::size_t i = 0; i < children_.size(); ++i) {
        if (children_[i].get() == &child) {
          return i;
        }
      }
      return children_.size();
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

    /** This actor's position, or nullptr when it is not movable. */
    virtual FlowGraphLocation* location() { return nullptr; }
    const FlowGraphLocation* location() const { return const_cast<Actor*>(this)->location(); }

    /** Whether a hit that lands on no child resolves to this actor. */
    virtual bool hittable() const { return true; }

    bool selected() const { return selected_; }
    void setSelected(bool selected) { selected_ = selected; }

    /** The id selecting this actor records, or nullopt when it is not selectable. */
    virtual std::optional<fluir::FullID> selectionId() const { return std::nullopt; }

    /** Re-derives this actor's `bounds()` from its model state, then its
     *  children's. Runs before hit-testing and drawing, never during either. */
    virtual void layout(const EditorContext& ctx) {
      for (const auto& child : children_) {
        child->layout(ctx);
      }
    }

    /** Handle a click event at `position`, in parent space. */
    virtual void onClick(Vec2 position) { }

    /** This actor's gestures, or nullptr when it cannot be grabbed. */
    virtual GestureHost* gestures() { return nullptr; }
    const GestureHost* gestures() const { return const_cast<Actor*>(this)->gestures(); }

    /** This actor's in-place editor, or nullptr when it has none. */
    virtual InlineEdit* editor() { return nullptr; }
    const InlineEdit* editor() const { return const_cast<Actor*>(this)->editor(); }

    /** Draws this actor into its parent's view, then its children into a nested
     *  view anchored (and clipped) to `bounds()`. */
    void draw(const Subview& parentView, const EditorContext& ctx) const {
      drawSelf(parentView, ctx);
      if (children_.empty()) {
        return;
      }
      if (clip_ == ClipChildren::No) {
        drawChildren(parentView, ctx);
      } else {
        const Subview view = parentView.child(bounds_);
        drawChildren(view, ctx);
      }
      drawOverlay(parentView, ctx);
    }

   protected:
    /** Draws this actor's own chrome, in the *parent's* view, so a frame's
     *  border is not clipped away by its own subview. */
    virtual void drawSelf(const Subview& parentView, const EditorContext& ctx) const { }
    /** Draws overlay components of this actor which will be drawn over all children. */
    virtual void drawOverlay(const Subview& parentView, const EditorContext& ctx) const { }

   private:
    void drawChildren(const Subview& view, const EditorContext& ctx) const {
      for (const auto& child : children_) {
        child->draw(view, ctx);
      }
    }

    Rect bounds_;
    ClipChildren clip_;
    bool selected_ = false;
    Actor* parent_ = nullptr;
    std::vector<std::unique_ptr<Actor>> children_;
  };

}  // namespace fluir::editor
