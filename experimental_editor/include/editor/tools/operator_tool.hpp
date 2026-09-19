#pragma once

#include "editor/tools/tool.hpp"

namespace fluir::editor {

  /** Left-press on an operator node opens a menu of its operators; a pick edits it. Never consumes. */
  class OperatorTool : public Tool {
   public:
    bool onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) override;
  };

}  // namespace fluir::editor
