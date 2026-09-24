#include "editor/view/draw/conditional.hpp"

#include "editor/core/renderer.hpp"
#include "editor/core/tree_path.hpp"

namespace fluir::editor::draw {
  namespace {

    // In grid units; deep enough that the conditional keeps room for a node under its header.
    constexpr Limits<Vec2i> SIZE_LIMITS{.lower = Vec2i{10, 10}, .upper = Vec2i{1000, 1000}};

  }  // namespace

  TerminalSet anchors(const pt::Conditional&, const Rect&, const EditorContext::Layout&) { return {}; }

  Color color(const pt::Conditional&, const EditorContext::Theme& theme) { return theme.conditionalNodeHeader; }

  Limits<Vec2i> sizeLimits(const pt::Conditional&) { return SIZE_LIMITS; }

  std::optional<Part> resizePart(const pt::Conditional&) { return Part::ResizeXY; }

  std::vector<FieldLabel> labels(const pt::Conditional&, const Rect&, const EditorContext::Layout&) { return {}; }

  void drawBody(const pt::Conditional&, const Rect& world, const Subview& view, const EditorContext& ctx) {
    view.renderer().fillRect(view.toScreen(world), ctx.theme.background);
  }

  std::string_view branchTag(fluir::ID branchId) { return branchId == ELSE_BRANCH_ID ? ELSE_TAG : THEN_TAG; }

  void drawFrame(const pt::Conditional& node, const Rect& frame, const Subview& view, const EditorContext& ctx) {
    const Rect header{frame.x, frame.y, frame.w, ctx.layout.headerH()};
    view.renderer().fillRect(view.toScreen(header), color(node, ctx.theme));
    view.renderer().drawRect(view.toScreen(frame), ctx.theme.border);
    drawSplitLabel(view, header, branchTag(THEN_BRANCH_ID), ctx);
  }

  void draw(const pt::Conditional& node, const Rect& world, const Subview& view, const EditorContext& ctx) {
    drawBody(node, world, view, ctx);
  }

}  // namespace fluir::editor::draw
