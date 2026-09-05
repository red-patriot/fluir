#pragma once

#include <functional>

#include "editor/components/button_actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/renderer.hpp"

namespace fluir::editor {

  /** App-level chrome bar drawn in identity screen-space. */
  class HeaderBar {
   public:
    explicit HeaderBar(std::function<void()> onExit);

    void draw(const EditorContext& ctx, Renderer& renderer, Vec2 outputSize);

    /** Screen-space hit test + dispatch; returns whether the click was consumed. */
    bool handleClick(Vec2 screenPos);

    const ButtonActor& exitButton() const { return exitButton_; }

   private:
    ButtonActor exitButton_;
  };

}  // namespace fluir::editor
