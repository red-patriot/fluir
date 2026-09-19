#pragma once

#include "editor/tools/tool.hpp"

namespace fluir::editor {

  /** Left-press selects whatever it hits; a miss clears. Never consumes. */
  class SelectTool : public Tool {
   public:
    bool onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) override;
  };

}  // namespace fluir::editor
