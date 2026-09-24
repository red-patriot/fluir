#include "editor/tools/select_tool.hpp"

#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  bool SelectTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    if (event.type != InputEvent::Type::MouseDown || event.button != InputEvent::Button::Left) {
      return false;
    }
    const Box* hit = hitAt(boxes, state.view.screenToWorld(event.pos));
    if (!hit) {
      state.selection = std::nullopt;
    } else {
      // A rail belongs to its function.
      state.selection = hit->part == Part::Rail ? parentOf(hit->path) : hit->path;
    }
    return false;  // tracked, never consumed
  }

}  // namespace fluir::editor
