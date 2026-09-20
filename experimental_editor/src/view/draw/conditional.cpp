#include "editor/view/draw/conditional.hpp"

#include "editor/core/renderer.hpp"

namespace fluir::editor::draw {

  PortSet anchors(const pt::Conditional&, const Rect&, const EditorContext::Layout&) { return {}; }

  Color color(const pt::Conditional&, const EditorContext::Theme& theme) { return theme.conditionalNodeHeader; }

  void drawBody(const pt::Conditional&, const Rect& world, const Subview& view, const EditorContext& ctx) {
    view.renderer().fillRect(view.toScreen(world), ctx.theme.background);
  }

  std::string_view branchTag(const pt::Conditional& conditional, const pt::Scope& scope) {
    return conditional.thenScope->id == scope.id ? THEN_TAG : ELSE_TAG;
  }

  void drawScope(
    const pt::Scope&, std::string_view tag, const Rect& branch, const Subview& view, const EditorContext& ctx) {
    const Rect header{branch.x, branch.y, branch.w, ctx.layout.headerH()};
    view.renderer().fillRect(view.toScreen(header), ctx.theme.conditionalNodeHeader);
    view.renderer().drawLine(
      view.toScreen(branch.topLeft()), view.toScreen(Vec2{branch.x + branch.w, branch.y}), ctx.theme.border);
    drawSplitLabel(view, header, tag, {}, ctx);
  }

  void drawFrame(const pt::Conditional&, const Rect& frame, const Subview& view, const EditorContext& ctx) {
    view.renderer().drawRect(view.toScreen(frame), ctx.theme.border);
  }

  void draw(const pt::Conditional& node, const Rect& world, const Subview& view, const EditorContext& ctx) {
    drawBody(node, world, view, ctx);
  }

}  // namespace fluir::editor::draw
