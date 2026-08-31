#pragma once

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  /** Walk `tree` and draw to renderer */
  void renderGraph(const pt::ParseTree& tree, const Viewport& view, Renderer& renderer);

}  // namespace fluir::editor
