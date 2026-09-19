#pragma once

#include "editor/tools/tool.hpp"

namespace fluir::editor {

  /** Right-press on empty background opens the top-level completions modal. Never consumes. */
  class CompletionTool : public Tool {
   public:
    bool onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) override;
  };

}  // namespace fluir::editor
