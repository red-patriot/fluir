#pragma once

#include <memory>

#include "editor/actors/actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/interaction.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"

namespace fluir::editor {

  /** A root Actor + a Viewport + an InteractionChain */
  class Layer {
   public:
    void setRoot(Actor& root) { root_ = &root; }
    void setViewport(Viewport viewport) { viewport_ = viewport; }

    Viewport& viewport() { return viewport_; }
    const Viewport& viewport() const { return viewport_; }

    void add(std::unique_ptr<Interaction> interaction) { chain_.add(std::move(interaction)); }

    /** Draws the root tree into a Subview spanning `outputRect`, composed with
     *  this layer's Viewport. */
    void draw(Renderer& renderer, const EditorContext& ctx, Rect outputRect) const;

    /** Returns the topmost actor containing `screenPos`, or nullptr if none contains it. */
    Actor* topmostAt(Vec2 screenPos) const;

    /** Routes one input event through this layer's interactions.
     * Returns true if one of them consumed the event. */
    bool dispatch(const InputEvent& event, EditorContext& ctx, Vec2 outputSize);

    /** Drops gesture state. */
    void reset() { chain_.reset(); }

    /** Gives up keyboard focus, leaving any live gesture alone. */
    void dropFocus() { chain_.dropFocus(); }

   private:
    Viewport viewport_;
    Actor* root_ = nullptr;
    InteractionChain chain_;
  };

}  // namespace fluir::editor
