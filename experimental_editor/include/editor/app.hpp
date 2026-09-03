#pragma once

#include "compiler/utility/diagnostic/sink.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/renderer.hpp"

namespace fluir::editor {

  /** Blocking view loop: fit `tree` into `viewportSize`, draw, then redraw on
   *  each input event until quit/Escape. Returns a process exit code (0 ok).
   *  SDL-free signature so P2d can drive it with a software renderer. */
  int run(const EditorContext& ctx, Renderer& renderer, Vec2 viewportSize);

}  // namespace fluir::editor
