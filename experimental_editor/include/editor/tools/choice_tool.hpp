#ifndef FLUIR_EDITOR_TOOLS_CHOICE_TOOL_HPP
#define FLUIR_EDITOR_TOOLS_CHOICE_TOOL_HPP

#include "editor/tools/tool.hpp"

namespace fluir::editor {

  /** Left-press on a picked field (an operator, a param or return type) opens a menu of its choices; a pick writes it.
   *  Never consumes. */
  class ChoiceTool : public Tool {
   public:
    bool onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) override;
  };

}  // namespace fluir::editor

#endif
