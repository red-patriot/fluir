#pragma once

#include <vector>

#include "editor/actors/actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"

namespace fluir::editor {

  /** A generic, ordered collection of non-owning Actor*, drawn/hit-tested through
   *  one Viewport. Gives any actor collection (e.g. HUD chrome) the same
   *  click+draw handling GraphScene already gives graph actors, without
   *  bespoke per-collection code. */
  class Layer {
   public:
    void setActors(std::vector<Actor*> actors) { actors_ = std::move(actors); }
    void setViewport(Viewport viewport) { viewport_ = viewport; }

    /** Draws every actor in paint order into a Subview spanning `outputRect`,
     *  composed with this layer's Viewport. */
    void draw(Renderer& renderer, const EditorContext& ctx, Rect outputRect) const;

    /** Topmost (last-painted) actor containing `screenPos` (converted through
     *  this layer's Viewport), or nullptr. Does not dispatch onClick itself --
     *  matches GraphScene::topmostAt's existing contract. */
    Actor* topmostAt(Vec2 screenPos) const;

    /** Routes one input event to this layer's actors.
     * Returns true if an actor consumed the event. */
    bool handleEvent(const InputEvent& event);

   private:
    Viewport viewport_;
    std::vector<Actor*> actors_;
  };

}  // namespace fluir::editor
