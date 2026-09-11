#pragma once

#include <functional>

#include "editor/components/button_actor.hpp"
#include "editor/components/toolbar_actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/module_editor.hpp"
#include "editor/core/renderer.hpp"

namespace fluir::editor {

  /** App-level chrome bar. */
  class HeaderBar : public ToolbarActor {
   public:
    HeaderBar(Renderer& renderer,
              std::function<void()> onSave,
              std::function<void()> onSaveAs,
              std::function<void()> onExit,
              ModuleEditor& editor);

    const ButtonActor& saveButton() const { return *saveButton_; }
    const ButtonActor& saveAsButton() const { return *saveAsButton_; }
    const ButtonActor& exitButton() const { return *exitButton_; }
    const ButtonActor& undoButton() const { return *undo_; }
    const ButtonActor& redoButton() const { return *redo_; }

   private:
    ModuleEditor& editor_;
    ButtonActor* saveButton_;
    ButtonActor* saveAsButton_;
    ButtonActor* exitButton_;
    ButtonActor* undo_;
    ButtonActor* redo_;
  };

}  // namespace fluir::editor
