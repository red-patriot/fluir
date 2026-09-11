#include "editor/pages/header_bar.hpp"

#include <utility>

#include "editor/core/module_editor.hpp"

namespace fluir::editor {

  HeaderBar::HeaderBar(Renderer& renderer,
                       std::function<void()> onSave,
                       std::function<void()> onSaveAs,
                       std::function<void()> onExit,
                       ModuleEditor& editor) :
    ToolbarActor(Rect{0, 0, 0, 0}, renderer),
    editor_(editor),
    saveButton_(&add("Save", std::move(onSave))),
    saveAsButton_(&add("Save As", std::move(onSaveAs))),
    exitButton_(&add("Exit", std::move(onExit), Align::Right)),
    undo_(&add("Undo",
               [this]() {
                 if (editor_.canUndo()) {
                   editor_.undo();
                 }
               })),
    redo_(&add("Redo", [this]() {
      if (editor_.canRedo()) {
        editor_.redo();
      }
    })) { }

}  // namespace fluir::editor
