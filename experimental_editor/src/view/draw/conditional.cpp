#include "editor/view/draw/conditional.hpp"

#include "editor/core/renderer.hpp"
#include "editor/core/tree_path.hpp"

namespace fluir::editor::draw {
  namespace {

    // In grid units; deep enough that the conditional keeps room for a node under its header.
    constexpr Limits<Vec2i> SIZE_LIMITS{.lower = Vec2i{10, 10}, .upper = Vec2i{1000, 1000}};
    // The branch arrow's side, in grid units.
    constexpr double BRANCH_ARROW_UNITS = 3;

    // Both tags are the same width, so the arrow sits at one place whichever branch shows.
    static_assert(THEN_TAG.size() == ELSE_TAG.size());

  }  // namespace

  TerminalSet anchors(const et::Conditional&, const Rect&, const EditorContext::Layout&) { return {}; }

  Color color(const et::Conditional&, const EditorContext::Theme& theme) { return theme.conditionalNodeHeader; }

  Limits<Vec2i> sizeLimits(const et::Conditional&) { return SIZE_LIMITS; }

  std::optional<Part> resizePart(const et::Conditional&) { return Part::ResizeXY; }

  std::vector<FieldLabel> labels(const et::Conditional&, const Rect&, const EditorContext::Layout&) { return {}; }

  void drawBody(const et::Conditional&, const Rect& world, const Subview& view, const EditorContext& ctx) {
    view.renderer().fillRect(view.toScreen(world), ctx.theme.background);
  }

  Rect branchArrowRect(const Rect& header, const EditorContext::Layout& layout) {
    const double side = BRANCH_ARROW_UNITS * layout.unitPx;
    const Rect tag = splitLabel(header, THEN_TAG, layout).tag;
    return {tag.x + tag.w, header.y + (header.h - side) / 2, side, side};
  }

  std::string_view branchTag(fluir::ID branchId) { return branchId == ELSE_BRANCH_ID ? ELSE_TAG : THEN_TAG; }

  void drawFrame(const et::Conditional& node, const Rect& frame, const Subview& view, const EditorContext& ctx) {
    const Rect header{frame.x, frame.y, frame.w, ctx.layout.headerH()};
    view.renderer().fillRect(view.toScreen(header), color(node, ctx.theme));
    view.renderer().drawRect(view.toScreen(frame), ctx.theme.border);
    drawSplitLabel(view, header, branchTag(node.annotation.shownBranch), ctx);
    drawImage(assets::branchArrowIcon(), branchArrowRect(header, ctx.layout), view, ctx.theme.text);
  }

  void draw(const et::Conditional& node, const Rect& world, const Subview& view, const EditorContext& ctx) {
    drawBody(node, world, view, ctx);
  }

}  // namespace fluir::editor::draw
