#pragma once

#include <vector>

#include "editor/tools/context_menu_tool.hpp"

namespace fluir::editor {

  /** "Add parameter" / "Add return" for a right-press on a function's header; empty elsewhere. */
  std::vector<MenuItem> functionHeaderItems(const Box& hit, const EditorState& state);

}  // namespace fluir::editor
