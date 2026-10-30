#include "editor/view/draw/function.hpp"

#include <algorithm>
#include <string>

#include "editor/view/graph_geometry.hpp"

namespace fluir::editor::draw {
  namespace {

    // In grid units.
    constexpr Limits<Vec2i> SIZE_LIMITS{.lower = Vec2i{15, 15}, .upper = Vec2i{1000, 1000}};

    // The parameter a rail shows; the return rail has none.
    const pt::FunctionParameter* paramOf(const Signature& fn, fluir::ID railId) {
      if (fn.input) {
        for (const auto& param : fn.input->parameters) {
          if (param.id == railId) {
            return &param;
          }
        }
      }
      return nullptr;
    }

    // `fn`'s parameters, ascending by index.
    std::vector<const pt::FunctionParameter*> sortedParameters(const Signature& fn) {
      std::vector<const pt::FunctionParameter*> out;
      if (fn.input) {
        for (const auto& param : fn.input->parameters) {
          out.push_back(&param);
        }
      }
      std::ranges::sort(out, {}, &pt::FunctionParameter::index);
      return out;
    }

    // The type name of `fn`'s param or return with id `railId`, or nullptr.
    const std::string* railTypeAt(const Signature& fn, fluir::ID railId) {
      if (fn.output && fn.output->ret && fn.output->ret->id == railId) {
        return &fn.output->ret->typeName;
      }
      const pt::FunctionParameter* param = paramOf(fn, railId);
      return param == nullptr ? nullptr : &param->typeName;
    }

    Rect headerOf(const Rect& frame, const EditorContext::Layout& layout) {
      return {frame.x, frame.y, frame.w, layout.headerH()};
    }

  }  // namespace

  TerminalSet anchors(const Signature& fn, fluir::ID railId, const Rect& rail) {
    const bool isReturn = fn.output && fn.output->ret && fn.output->ret->id == railId;
    const double midY = rail.y + rail.h * 0.5;
    return isReturn ? TerminalSet{{Vec2{rail.x, midY}}, {}} : TerminalSet{{}, {Vec2{rail.x + rail.w, midY}}};
  }

  Rect railRect(const Signature& fn, fluir::ID railId, const Rect& frame, const EditorContext::Layout& layout) {
    const Vec2 origin = bodyOrigin(frame.topLeft(), layout.headerH());
    if (fn.output && fn.output->ret && fn.output->ret->id == railId) {
      return {
        origin.x + frame.w - layout.returnInsetUnits * layout.unitPx, origin.y, layout.railStep(), layout.railStep()};
    }
    const std::vector<const pt::FunctionParameter*> params = sortedParameters(fn);
    for (std::size_t row = 0; row < params.size(); ++row) {
      if (params[row]->id == railId) {
        return {origin.x, origin.y + static_cast<double>(row) * layout.railStep(), layout.paramW(), layout.railStep()};
      }
    }
    return {};
  }

  Color color(const Signature&, const EditorContext::Theme& theme) { return theme.funcDeclHeader; }

  Limits<Vec2i> sizeLimits(const Signature&) { return SIZE_LIMITS; }

  std::vector<FieldLabel> labels(const Signature&, const Rect& frame, const EditorContext::Layout& layout) {
    return {{{Field::Kind::Name}, splitLabel(headerOf(frame, layout), FN_TAG, layout).text}};
  }

  std::vector<FieldLabel> labels(const Signature& fn,
                                 fluir::ID railId,
                                 const Rect& rail,
                                 const EditorContext::Layout& layout) {
    const std::string* typeName = railTypeAt(fn, railId);
    if (typeName == nullptr) {
      return {};
    }
    const SplitLabel split = splitLabel(rail, *typeName, layout);
    const pt::FunctionParameter* param = paramOf(fn, railId);
    if (param == nullptr) {
      return {{{Field::Kind::ReturnType}, split.tag}};
    }
    return {{{Field::Kind::ParamType, param->index}, split.tag}, {{Field::Kind::ParamName, param->index}, split.text}};
  }

}  // namespace fluir::editor::draw
