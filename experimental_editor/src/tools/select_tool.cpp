#include "editor/tools/select_tool.hpp"

namespace fluir::editor {

  bool SelectTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    if (event.type != InputEvent::Type::MouseDown || event.button != InputEvent::Button::Left) {
      return false;
    }
    const Box* hit = hitAt(boxes, state.view.screenToWorld(event.pos));
    state.selection = hit == nullptr ? std::nullopt : std::optional<FullID>{hit->path};
    return false;  // tracked, never consumed
  }

}  // namespace fluir::editor
