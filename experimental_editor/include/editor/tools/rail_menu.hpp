#pragma once

#include <vector>

#include "editor/tools/context_menu_tool.hpp"

namespace fluir::editor {

  /** "Delete" for a right-press on a function's parameter or return rail; empty elsewhere. */
  std::vector<MenuItem> railItems(const Box& hit, Vec2 world, const EditorState& state);

}  // namespace fluir::editor
