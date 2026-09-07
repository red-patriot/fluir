#include "editor/actors/function_decl_actor.hpp"

#include <fmt/format.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  void FunctionDeclActor::onClick(Vec2) { lastClickSummary_ = fmt::format("function {}", name_); }

  bool FunctionDeclActor::onDragStart(const EditorContext& ctx, Vec2 worldPos) {
    return drag_.onDragStart(ctx, worldPos);
  }

  void FunctionDeclActor::onDrag(const EditorContext& ctx, Vec2 position, Vec2 delta) {
    return drag_.onDrag(ctx, position, delta);
  }

  void FunctionDeclActor::draw(const Subview& frame, const EditorContext& ctx) const {
    const double x = static_cast<double>(location_.x) * ctx.layout.unitPx;
    const double y = static_cast<double>(location_.y) * ctx.layout.unitPx;
    const double w = static_cast<double>(location_.width) * ctx.layout.unitPx;
    const double h = static_cast<double>(location_.height) * ctx.layout.unitPx;
    const double headerH = ctx.layout.headerH();

    const auto headerRect = Rect{x, y, w, headerH};

    frame.renderer().fillRect(frame.toScreen(headerRect), ctx.theme.funcDeclHeader);
    frame.renderer().drawRect(frame.toScreen(Rect{x, y, w, h}), ctx.theme.border);
    frame.renderer().drawText(
      frame.toScreen(Vec2{x + ctx.layout.textPad, y + ctx.layout.textPad}), name_, ctx.theme.text);

    drag_.draw(frame, ctx, headerRect);
  }

}  // namespace fluir::editor
