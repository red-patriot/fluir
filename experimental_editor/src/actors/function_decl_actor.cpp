#include "editor/actors/function_decl_actor.hpp"

#include <fmt/format.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  void FunctionDeclActor::onClick(Vec2) { lastClickSummary_ = fmt::format("function {}", name_); }

  void FunctionDeclActor::draw(const Subview& frame, const EditorContext& ctx) const {
    const double w = static_cast<double>(location_.width) * ctx.layout.unitPx;
    const double h = static_cast<double>(location_.height) * ctx.layout.unitPx;
    const double headerH = ctx.layout.headerH();

    frame.renderer().fillRect(frame.toScreen(Rect{0, 0, w, headerH}), ctx.theme.funcDeclHeader);
    frame.renderer().drawRect(frame.toScreen(Rect{0, 0, w, h}), ctx.theme.border);
    frame.renderer().drawText(frame.toScreen(Vec2{ctx.layout.textPad, ctx.layout.textPad}), name_, ctx.theme.text);
  }

  PortSet FunctionDeclActor::ports(const EditorContext&) const { return PortSet{}; }

}  // namespace fluir::editor
