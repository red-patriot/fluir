#include "editor/view/draw/comment.hpp"

#include "editor/core/renderer.hpp"

namespace fluir::editor {

  Rect commentTextRect(Rect world, const EditorContext::Layout& layout) {
    const double pad = layout.textPad;
    return {world.x + pad, world.y + pad, world.w - 2 * pad, world.h - 2 * pad};
  }

  namespace draw {

    PortSet anchors(const pt::Comment&, const Rect&, const EditorContext::Layout&) { return {}; }

    Color color(const pt::Comment&, const EditorContext::Theme& theme) { return theme.commentNode; }

    void draw(const pt::Comment& comment, const Rect& world, const Subview& view, const EditorContext& ctx) {
      drawShell(world, color(comment, ctx.theme), view, ctx);
      if (comment.text.empty()) {
        return;
      }
      const Rect textRect = commentTextRect(world, ctx.layout);
      const Subview clipped = view.child(textRect);
      clipped.renderer().drawTextWrapped(
        clipped.toScreen(Rect{0, 0, textRect.w, textRect.h}), comment.text, clipped.composed().scale, ctx.theme.text);
    }

  }  // namespace draw

}  // namespace fluir::editor
