#include "editor/tools/popup_tool.hpp"

namespace fluir::editor {

  bool PopupTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box>) {
    if (!state.popup) {
      return false;
    }
    if (!state.popup->onEvent(event, state)) {
      state.popup.reset();
    }
    return true;
  }

  void PopupTool::draw(const Subview& view, const EditorState& state, std::span<const Box>) const {
    if (state.popup) {
      state.popup->draw(view.renderer(), state.ctx);
    }
  }

}  // namespace fluir::editor
