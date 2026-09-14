#include "editor/tools/tool.hpp"

namespace fluir::editor {

  bool ToolChain::dispatch(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    if (captured_ != nullptr) {
      const bool consumed = captured_->onEvent(event, state, boxes);
      if (!captured_->capturing()) {
        captured_ = nullptr;
      }
      return consumed;
    }
    for (const auto& tool : tools_) {
      if (tool->onEvent(event, state, boxes)) {
        if (tool->capturing()) {
          captured_ = tool.get();
        }
        return true;
      }
    }
    return false;
  }

  void ToolChain::cancel(EditorState& state) {
    captured_ = nullptr;
    for (const auto& tool : tools_) {
      tool->cancel(state);
    }
  }

  void ToolChain::draw(const Subview& view, const EditorState& state, std::span<const Box> boxes) const {
    // Reverse priority, so the first tool paints on top.
    for (auto it = tools_.rbegin(); it != tools_.rend(); ++it) {
      (*it)->draw(view, state, boxes);
    }
  }

}  // namespace fluir::editor
