#include "editor/actors/function_decl_actor.hpp"

#include <memory>

#include <fmt/format.h>

#include "editor/actors/selection_outline.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/graph_geometry.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {
  namespace {
    constexpr Limits<Vec2i> SIZE_UNITS{.lower = Vec2i{15, 15}, .upper = Vec2i{1000, 1000}};
  }  // namespace

  FunctionDeclActor::FunctionDeclActor(const pt::FunctionDecl& decl, Rect bounds) :
    Actor(bounds),
    functionId_(decl.id),
    location_(decl.location),
    name_(decl.name),
    // The header grip is listed first, so it keeps every pixel it shares with the corner.
    gestures_(*this, {dragGrip(actorHeader), xyResizeGrip(SIZE_UNITS)}, SIZE_UNITS),
    body_(&add(std::make_unique<ContainerActor>(Rect{0, 0, bounds.w, bounds.h}))) { }

  void FunctionDeclActor::layout(const EditorContext& ctx) {
    const Rect frame = localRect(previewLocation(), ctx.layout.unitPx);
    setBounds(frame);
    body_->setBounds(Rect{0, ctx.layout.headerH(), frame.w, frame.h});
    if (return_ != nullptr) {
      return_->setFrameWidth(previewLocation().width);
    }
    Actor::layout(ctx);
  }

  PortActor* FunctionDeclActor::port(fluir::ID portId) const {
    const auto it = ports_.find(portId);
    return it == ports_.end() ? nullptr : it->second;
  }

  void FunctionDeclActor::onClick(Vec2) { lastClickSummary_ = fmt::format("function {}", name_); }

  Rect FunctionDeclActor::headerRect(const EditorContext& ctx) const {
    return Rect{bounds().x, bounds().y, bounds().w, ctx.layout.headerH()};
  }

  void FunctionDeclActor::drawSelf(const Subview& parentView, const EditorContext& ctx) const {
    parentView.renderer().fillRect(parentView.toScreen(bounds()), ctx.theme.background);
  }

  void FunctionDeclActor::drawOverlay(const Subview& parentView, const EditorContext& ctx) const {
    const Rect header = headerRect(ctx);

    parentView.renderer().fillRect(parentView.toScreen(header), ctx.theme.funcDeclHeader);
    parentView.renderer().drawRect(parentView.toScreen(bounds()), ctx.theme.border);
    parentView.renderer().drawText(
      parentView.toScreen(bounds().topLeft() + Vec2{ctx.layout.textPad, ctx.layout.textPad}), name_, ctx.theme.text);

    gestures_.draw(parentView, ctx);

    if (selected()) {
      drawSelectionOutline(parentView, ctx, bounds());
    }
  }

}  // namespace fluir::editor
