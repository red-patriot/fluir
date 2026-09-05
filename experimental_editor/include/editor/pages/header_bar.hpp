#pragma once

#include <functional>
#include <vector>

#include "editor/actors/actor.hpp"
#include "editor/components/button_actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/renderer.hpp"

namespace fluir::editor {

  /** App-level chrome bar drawn in identity screen-space. */
  class HeaderBar {
   public:
    explicit HeaderBar(std::function<void()> onExit);

    /** Repositions every button for the current output size. */
    void layout(const EditorContext& ctx, Vec2 outputSize);

    /** Draws the bar. */
    void drawChrome(const EditorContext& ctx, Renderer& renderer, Vec2 outputSize) const;

    /** This bar's buttons, for a caller to feed into its own Layer. */
    std::vector<Actor*> actors() { return {&exitButton_}; }

    const ButtonActor& exitButton() const { return exitButton_; }

   private:
    ButtonActor exitButton_;
  };

}  // namespace fluir::editor
