#pragma once

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/renderer.hpp"

namespace fluir::editor {

  /** Blocking view loop: fit `tree` into `viewportSize`, draw, then redraw on
   *  each input event until quit/Escape. Returns a process exit code (0 ok).
   *  SDL-free signature so P2d can drive it with a software renderer. */
  int run(const pt::ParseTree& tree, Renderer& renderer, Vec2 viewportSize);

}  // namespace fluir::editor
