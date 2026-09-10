#pragma once

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  /** Two concentric rects just outside `local`, marking it as the selected actor. */
  void drawSelectionOutline(const Subview& view, const EditorContext& ctx, Rect local);

}  // namespace fluir::editor
