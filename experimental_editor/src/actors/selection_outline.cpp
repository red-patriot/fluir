#include "editor/actors/selection_outline.hpp"

#include "editor/core/renderer.hpp"

namespace fluir::editor {
  namespace {

    Rect outset(Rect local, double pad) { return {local.x - pad, local.y - pad, local.w + 2 * pad, local.h + 2 * pad}; }

  }  // namespace

  // Renderer::drawRect has no thickness, so two concentric rects stand in for
  // a 2px outline.
  void drawSelectionOutline(const Subview& view, const EditorContext& ctx, Rect local) {
    const double pad = ctx.layout.selectionPad;
    view.renderer().drawRect(view.toScreen(outset(local, pad)), ctx.theme.border);
    view.renderer().drawRect(view.toScreen(outset(local, pad + 1.0)), ctx.theme.border);
  }

}  // namespace fluir::editor
