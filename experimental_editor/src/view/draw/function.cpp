#include "editor/view/draw/function.hpp"

#include <string>

#include "editor/core/renderer.hpp"
#include "editor/core/tree_path.hpp"

namespace fluir::editor::draw {
  namespace {

    // In grid units.
    constexpr Limits<Vec2i> SIZE_LIMITS{.lower = Vec2i{15, 15}, .upper = Vec2i{1000, 1000}};

    // The parameter a rail shows; the return rail has none.
    const et::FunctionDecl::Parameter* paramOf(const et::FunctionDecl& fn, fluir::ID railId) {
      if (fn.input) {
        for (const auto& param : fn.input->parameters) {
          if (param.id == railId) {
            return &param;
          }
        }
      }
      return nullptr;
    }

    Rect headerOf(const Rect& frame, const EditorContext::Layout& layout) {
      return {frame.x, frame.y, frame.w, layout.headerH()};
    }

  }  // namespace

  TerminalSet anchors(const et::FunctionDecl& fn, fluir::ID railId, const Rect& rail) {
    const bool isReturn = fn.output && fn.output->ret && fn.output->ret->id == railId;
    const double midY = rail.y + rail.h * 0.5;
    return isReturn ? TerminalSet{{Vec2{rail.x, midY}}, {}} : TerminalSet{{}, {Vec2{rail.x + rail.w, midY}}};
  }

  Color color(const et::FunctionDecl&, const EditorContext::Theme& theme) { return theme.funcDeclHeader; }

  Limits<Vec2i> sizeLimits(const et::FunctionDecl&) { return SIZE_LIMITS; }

  std::vector<FieldLabel> labels(const et::FunctionDecl&, const Rect& frame, const EditorContext::Layout& layout) {
    return {{{Field::Kind::Name}, splitLabel(headerOf(frame, layout), FN_TAG, layout).text}};
  }

  std::vector<FieldLabel> labels(const et::FunctionDecl& fn,
                                 fluir::ID railId,
                                 const Rect& rail,
                                 const EditorContext::Layout& layout) {
    const std::string* typeName = railTypeAt(fn, railId);
    if (typeName == nullptr) {
      return {};
    }
    const SplitLabel split = splitLabel(rail, *typeName, layout);
    const et::FunctionDecl::Parameter* param = paramOf(fn, railId);
    if (param == nullptr) {
      return {{{Field::Kind::ReturnType}, split.tag}};
    }
    return {{{Field::Kind::ParamType, param->index}, split.tag}, {{Field::Kind::ParamName, param->index}, split.text}};
  }

  void drawBody(const et::FunctionDecl&, const Rect& world, const Subview& view, const EditorContext& ctx) {
    view.renderer().fillRect(view.toScreen(world), ctx.theme.background);
  }

  void drawRail(
    const et::FunctionDecl& fn, fluir::ID railId, const Rect& rail, const Subview& view, const EditorContext& ctx) {
    const std::string* typeName = railTypeAt(fn, railId);
    drawShell(rail, color(fn, ctx.theme), view, ctx);
    drawSplitLabel(view, rail, typeName == nullptr ? std::string_view{} : *typeName, ctx);
    if (const et::FunctionDecl::Parameter* param = paramOf(fn, railId)) {
      drawTitle(param->name, labels(fn, railId, rail, ctx.layout).back().rect, view, ctx);
    }
    drawTerminalDots(anchors(fn, railId, rail), view, ctx);
  }

  void drawFrame(const et::FunctionDecl& fn, const Rect& frame, const Subview& view, const EditorContext& ctx) {
    const Rect header = headerOf(frame, ctx.layout);
    view.renderer().fillRect(view.toScreen(header), color(fn, ctx.theme));
    view.renderer().drawRect(view.toScreen(frame), ctx.theme.border);
    drawSplitLabel(view, header, FN_TAG, ctx);
    drawTitle(fn.name, labels(fn, frame, ctx.layout).front().rect, view, ctx);
  }

}  // namespace fluir::editor::draw
