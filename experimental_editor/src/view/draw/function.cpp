#include "editor/view/draw/function.hpp"

#include <string>

#include "editor/core/node_access.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/view/graph_geometry.hpp"

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

  Rect railRect(const et::FunctionDecl& fn, fluir::ID railId, const Rect& frame, const EditorContext::Layout& layout) {
    const Vec2 origin = bodyOrigin(frame.topLeft(), layout.headerH());
    if (fn.output && fn.output->ret && fn.output->ret->id == railId) {
      return {origin.x + (fn.location.width - layout.returnInsetUnits) * layout.unitPx,
              origin.y,
              layout.railStep(),
              layout.railStep()};
    }
    const std::vector<const et::FunctionDecl::Parameter*> params = sortedParameters(fn);
    for (std::size_t row = 0; row < params.size(); ++row) {
      if (params[row]->id == railId) {
        return {origin.x, origin.y + static_cast<double>(row) * layout.railStep(), layout.paramW(), layout.railStep()};
      }
    }
    return {};
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

}  // namespace fluir::editor::draw
