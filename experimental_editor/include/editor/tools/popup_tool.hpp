#pragma once

#include "editor/tools/tool.hpp"

namespace fluir::editor {

  /** Hosts `EditorState::popup`: an open popup takes and consumes every event. First in the chain. */
  class PopupTool : public Tool {
   public:
    bool onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) override;
    void cancel(EditorState& state) override { state.popup.reset(); }
    void draw(const Subview& view, const EditorState& state, std::span<const Box> boxes) const override;
  };

}  // namespace fluir::editor
