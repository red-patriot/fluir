#pragma once

#include "editor/tools/tool.hpp"

namespace fluir::editor {

  /** Hosts `EditorState::popup`: an open popup takes and consumes every event. First in the chain.
   *  The page draws the popup itself, over the header and unclipped. */
  class PopupTool : public Tool {
   public:
    bool onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) override;
    void cancel(EditorState& state) override { state.popup.reset(); }
  };

}  // namespace fluir::editor
