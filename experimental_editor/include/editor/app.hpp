#pragma once

#include "compiler/utility/diagnostic/sink.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/renderer.hpp"

namespace fluir::editor {

  /**  Runs the main app. Returns a process exit code (0 ok).. */
  int run(EditorContext& ctx, Renderer& renderer);

}  // namespace fluir::editor
