#include "editor/actors/function_decl_actor.hpp"

#include <memory>

#include <fmt/format.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/graph_geometry.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  FunctionDeclActor::FunctionDeclActor(const pt::FunctionDecl& decl, Rect bounds) :
    Actor(bounds),
    functionId_(decl.id),
    location_(decl.location),
    name_(decl.name),
    body_(&add(std::make_unique<ContainerActor>(Rect{0, 0, bounds.w, bounds.h}))) { }

  void FunctionDeclActor::layout(const EditorContext& ctx) {
    const Rect frame = localRect(location_, ctx.layout.unitPx);
    setBounds(frame);
    body_->setBounds(Rect{0, ctx.layout.headerH(), frame.w, frame.h});
    if (return_ != nullptr) {
      return_->setFrameWidth(location_.width);
    }
    Actor::layout(ctx);
  }

  void FunctionDeclActor::onClick(Vec2) { lastClickSummary_ = fmt::format("function {}", name_); }

  bool FunctionDeclActor::onDragStart(const EditorContext& ctx, Vec2 position) {
    return drag_.onDragStart(ctx, position, location_, headerRect(ctx));
  }

  void FunctionDeclActor::onDrag(const EditorContext& ctx, Vec2, Vec2 delta) { drag_.onDrag(ctx, delta, location_); }

  Rect FunctionDeclActor::headerRect(const EditorContext& ctx) const {
    return Rect{bounds().x, bounds().y, bounds().w, ctx.layout.headerH()};
  }

  void FunctionDeclActor::drawSelf(const Subview& parentView, const EditorContext& ctx) const {
    parentView.renderer().fillRect(parentView.toScreen(bounds()), ctx.theme.background);
    const Rect header = headerRect(ctx);

    parentView.renderer().fillRect(parentView.toScreen(header), ctx.theme.funcDeclHeader);
    parentView.renderer().drawRect(parentView.toScreen(bounds()), ctx.theme.border);
    parentView.renderer().drawText(
      parentView.toScreen(bounds().topLeft() + Vec2{ctx.layout.textPad, ctx.layout.textPad}), name_, ctx.theme.text);

    drag_.draw(parentView, ctx, location_, header);
  }

}  // namespace fluir::editor
