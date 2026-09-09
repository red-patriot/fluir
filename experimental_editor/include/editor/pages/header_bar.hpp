#pragma once

#include <functional>

#include "editor/components/button_actor.hpp"
#include "editor/components/toolbar_actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/renderer.hpp"

namespace fluir::editor {

  /** App-level chrome bar. */
  class HeaderBar : public ToolbarActor {
   public:
    HeaderBar(Renderer& renderer,
              std::function<void()> onSave,
              std::function<void()> onSaveAs,
              std::function<void()> onExit);

    const ButtonActor& saveButton() const { return *saveButton_; }
    const ButtonActor& saveAsButton() const { return *saveAsButton_; }
    const ButtonActor& exitButton() const { return *exitButton_; }

   private:
    ButtonActor* saveButton_;
    ButtonActor* saveAsButton_;
    ButtonActor* exitButton_;
  };

}  // namespace fluir::editor
