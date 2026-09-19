#pragma once

#include "editor/tools/tool.hpp"

namespace fluir::editor {

  /** Left-press on a bool constant's toggle square flips its value. */
  class BoolToggleTool : public Tool {
   public:
    bool onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) override;
  };

}  // namespace fluir::editor
