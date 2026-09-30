#ifndef FLUIR_EDITOR_TOOLS_PORT_MENU_HPP
#define FLUIR_EDITOR_TOOLS_PORT_MENU_HPP

#include <vector>

#include "editor/tools/context_menu_tool.hpp"

namespace fluir::editor {

  /** "Delete port" for a right-click on a conditional's non-condition port. */
  std::vector<MenuItem> portItems(const Box& hit, const EditorState& state);

}  // namespace fluir::editor

#endif
