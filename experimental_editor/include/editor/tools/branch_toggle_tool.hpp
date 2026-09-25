#ifndef FLUIR_EDITOR_TOOLS_BRANCH_TOGGLE_TOOL_HPP
#define FLUIR_EDITOR_TOOLS_BRANCH_TOGGLE_TOOL_HPP

#include "editor/tools/tool.hpp"

namespace fluir::editor {

  /** Left-press on the arrow in a conditional's header band switches which branch it shows. View state only, so the
   *  press never enters undo. */
  class BranchToggleTool : public Tool {
   public:
    bool onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) override;
  };

}  // namespace fluir::editor

#endif
