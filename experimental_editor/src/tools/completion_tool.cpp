#include "editor/tools/completion_tool.hpp"

#include <memory>
#include <utility>
#include <vector>

#include "editor/tools/completion_modal.hpp"
#include "editor/tools/menu_popup.hpp"

namespace fluir::editor {

  bool CompletionTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    if (event.type != InputEvent::Type::MouseDown || event.button != InputEvent::Button::Right ||
        hitAt(boxes, state.view.screenToWorld(event.pos)) != nullptr) {
      return false;
    }
    std::vector<Completion> completions = state.intelligence.completions(state.editor.tree(), {});
    if (completions.empty()) {
      return false;
    }
    state.popup = std::make_unique<CompletionModal>(std::move(completions), popupBounds(state), state.text);
    return false;  // tracked, never consumed
  }

}  // namespace fluir::editor
