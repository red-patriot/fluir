#pragma once

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  /** Walk `tree` and draw to renderer */
  void renderGraph(const pt::ParseTree& tree, const Viewport& view, Renderer& renderer);

  /** World-space axis-aligned bounding box of every function frame in `tree`
   *  ({0,0,0,0} when there are no functions). Used to fit the view. */
  Rect graphBounds(const pt::ParseTree& tree);

}  // namespace fluir::editor
