#include "editor/pages/header_bar.hpp"

#include <utility>

namespace fluir::editor {

  HeaderBar::HeaderBar(Renderer& renderer,
                       std::function<void()> onSave,
                       std::function<void()> onSaveAs,
                       std::function<void()> onExit) :
    ToolbarActor(Rect{0, 0, 0, 0}, renderer),
    saveButton_(&add("Save", std::move(onSave))),
    saveAsButton_(&add("Save As", std::move(onSaveAs))),
    exitButton_(&add("Exit", std::move(onExit), Align::Right)) { }

}  // namespace fluir::editor
