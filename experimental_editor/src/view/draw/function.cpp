#include "editor/view/draw/function.hpp"

#include <string>

#include "editor/assets/images.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/tree_path.hpp"

namespace fluir::editor::draw {
  namespace {

    // A parameter rail's name; the return rail has none.
    std::string_view railName(const pt::FunctionDecl& fn, fluir::ID id) {
      if (fn.input) {
        for (const auto& param : fn.input->parameters) {
          if (param.id == id) {
            return param.name;
          }
        }
      }
      return {};
    }

  }  // namespace

  PortSet anchors(const pt::FunctionDecl& fn, fluir::ID railId, const Rect& rail) {
    const bool isReturn = fn.output && fn.output->ret && fn.output->ret->id == railId;
    const double midY = rail.y + rail.h * 0.5;
    return isReturn ? PortSet{{Vec2{rail.x, midY}}, {}} : PortSet{{}, {Vec2{rail.x + rail.w, midY}}};
  }

  Color color(const pt::FunctionDecl&, const EditorContext::Theme& theme) { return theme.funcDeclHeader; }

  void drawBody(const pt::FunctionDecl&, const Rect& world, const Subview& view, const EditorContext& ctx) {
    view.renderer().fillRect(view.toScreen(world), ctx.theme.background);
  }

  void drawRail(
    const pt::FunctionDecl& fn, fluir::ID railId, const Rect& rail, const Subview& view, const EditorContext& ctx) {
    const std::string* typeName = railTypeAt(fn, railId);
    drawShell(rail, color(fn, ctx.theme), view, ctx);
    drawSplitLabel(view, rail, typeName == nullptr ? std::string_view{} : *typeName, railName(fn, railId), ctx);
    drawPortDots(anchors(fn, railId, rail), view, ctx);
  }

  void drawMoveGrip(const pt::FunctionDecl&, const Rect& grip, const Subview& view, const EditorContext& ctx) {
    drawImage(assets::dragHandle(), grip, view, ctx.theme.border);
  }

  void drawFrame(const pt::FunctionDecl& fn, const Rect& frame, const Subview& view, const EditorContext& ctx) {
    const Rect header{frame.x, frame.y, frame.w, ctx.layout.headerH()};
    view.renderer().fillRect(view.toScreen(header), color(fn, ctx.theme));
    view.renderer().drawRect(view.toScreen(frame), ctx.theme.border);
    drawSplitLabel(view, header, FN_TAG, fn.name, ctx);
  }

}  // namespace fluir::editor::draw
