#include "editor/view/draw/function.hpp"

#include <algorithm>
#include <optional>
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

    // The position of the parameter a rail shows; the return rail has none.
    std::optional<int> positionOf(const Signature& fn, fluir::ID railId) {
      if (fn.input) {
        const auto& params = fn.input->parameters;
        const auto it = std::ranges::find(params, railId, &pt::FunctionParameter::id);
        if (it != params.end()) {
          return static_cast<int>(it - params.begin());
        }
      }
      return std::nullopt;
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
    const std::optional<int> row = positionOf(fn, railId);
    if (!row) {
      return {};
    }
    return {origin.x, origin.y + static_cast<double>(*row) * layout.railStep(), layout.paramW(), layout.railStep()};
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
    const std::optional<int> position = positionOf(fn, railId);
    if (!position) {
      return {{{Field::Kind::ReturnType}, split.tag, FieldLook::Tag}};
    }
    return {{{Field::Kind::ParamType, *position}, split.tag, FieldLook::Tag},
            {{Field::Kind::ParamName, *position}, split.text}};
  }

}  // namespace fluir::editor::draw
