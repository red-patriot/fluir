#ifndef FLUIR_EDITOR_TOOLS_CONDITIONAL_HEADER_MENU_HPP
#define FLUIR_EDITOR_TOOLS_CONDITIONAL_HEADER_MENU_HPP

#include <vector>

#include "editor/tools/context_menu_tool.hpp"

namespace fluir::editor {

  /** "Add input" / "Add output" for a right-press on a conditional's header. */
  std::vector<MenuItem> conditionalHeaderItems(const Box& hit, const EditorState& state);

}  // namespace fluir::editor

#endif
