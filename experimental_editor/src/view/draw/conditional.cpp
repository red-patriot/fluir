#include "editor/view/draw/conditional.hpp"

#include "editor/core/renderer.hpp"

namespace fluir::editor::draw {

  PortSet anchors(const pt::Conditional&, const Rect&, const EditorContext::Layout&) { return {}; }

  Color color(const pt::Conditional&, const EditorContext::Theme& theme) { return theme.conditionalNodeHeader; }

  void drawBody(const pt::Conditional&, const Rect& world, const Subview& view, const EditorContext& ctx) {
    view.renderer().fillRect(view.toScreen(world), ctx.theme.background);
  }

  void drawScope(const pt::Scope&, const Rect& branch, const Subview& view, const EditorContext& ctx) {
    view.renderer().drawLine(
      view.toScreen(branch.topLeft()), view.toScreen(Vec2{branch.x + branch.w, branch.y}), ctx.theme.border);
  }

  void drawFrame(const pt::Conditional& node, const Rect& frame, const Subview& view, const EditorContext& ctx) {
    const Rect header{frame.x, frame.y, frame.w, ctx.layout.headerH()};
    view.renderer().fillRect(view.toScreen(header), color(node, ctx.theme));
    view.renderer().drawRect(view.toScreen(frame), ctx.theme.border);
    drawSplitLabel(view, header, IF_TAG, {}, ctx);
  }

  void draw(const pt::Conditional& node, const Rect& world, const Subview& view, const EditorContext& ctx) {
    drawBody(node, world, view, ctx);
  }

}  // namespace fluir::editor::draw
