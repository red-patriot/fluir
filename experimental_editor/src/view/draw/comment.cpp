#include "editor/view/draw/comment.hpp"

#include <algorithm>
#include <string_view>

#include "editor/core/renderer.hpp"

namespace fluir::editor {
  namespace {

    constexpr std::string_view COMMENT_TAG = "//";
    // In grid units; tall enough that the corner grip never overlaps the move grip.
    constexpr Limits<Vec2i> SIZE_LIMITS{.lower = Vec2i{8, 8}, .upper = Vec2i{1000, 1000}};

  }  // namespace

  Rect commentBodyRect(Rect world, const EditorContext::Layout& layout) {
    const double headerH = layout.headerH();
    return {world.x, world.y + headerH, world.w, std::max(0.0, world.h - headerH)};
  }

  Rect commentTextRect(Rect body, const EditorContext::Layout& layout) {
    const double pad = layout.textPad;
    return {body.x + pad, body.y + pad, body.w - 2 * pad, body.h - 2 * pad};
  }

  namespace draw {

    TerminalSet anchors(const et::Comment&, const Rect&, const EditorContext::Layout&) { return {}; }

    Color color(const et::Comment&, const EditorContext::Theme& theme) { return theme.commentNode; }

    Limits<Vec2i> sizeLimits(const et::Comment&) { return SIZE_LIMITS; }

    std::optional<Part> resizePart(const et::Comment&) { return Part::ResizeXY; }

    std::vector<FieldLabel> labels(const et::Comment&, const Rect& world, const EditorContext::Layout& layout) {
      return {{{Field::Kind::Text}, commentBodyRect(world, layout)}};
    }

    void draw(const et::Comment& comment, const Rect& world, const Subview& view, const EditorContext& ctx) {
      drawShell(world, color(comment, ctx.theme), view, ctx);
      drawSplitLabel(view, {world.x, world.y, world.w, ctx.layout.headerH()}, COMMENT_TAG, ctx);
      if (comment.text.empty()) {
        return;
      }
      const Rect textRect = commentTextRect(labels(comment, world, ctx.layout).front().rect, ctx.layout);
      const Subview clipped = view.child(textRect);
      clipped.renderer().drawTextWrapped(
        clipped.toScreen(Rect{0, 0, textRect.w, textRect.h}), comment.text, clipped.composed().scale, ctx.theme.text);
    }

  }  // namespace draw

}  // namespace fluir::editor
