#include "editor/view/draw/function.hpp"

#include <algorithm>
#include <optional>
#include <string>

#include "editor/view/graph_geometry.hpp"

namespace fluir::editor::draw {
  namespace {

    // In grid units.
    constexpr Limits<Vec2i> SIZE_LIMITS{.lower = Vec2i{15, 15}, .upper = Vec2i{1000, 1000}};
    constexpr Vec2i DEFAULT_SIZE{40, 30};

    // The parameter a rect shows.
    const pt::FunctionParameter* paramOf(const Signature& fn, fluir::ID id) {
      if (fn.input) {
        for (const auto& param : fn.input->parameters) {
          if (param.id == id) {
            return &param;
          }
        }
      }
      return nullptr;
    }

    // The position of the parameter a rect shows.
    std::optional<int> positionOf(const Signature& fn, fluir::ID id) {
      if (fn.input) {
        const auto& params = fn.input->parameters;
        const auto it = std::ranges::find(params, id, &pt::FunctionParameter::id);
        if (it != params.end()) {
          return static_cast<int>(it - params.begin());
        }
      }
      return std::nullopt;
    }

    // The type name of `fn`'s param or return with id `id` or nullptr.
    const std::string* typeNameAt(const Signature& fn, fluir::ID id) {
      if (fn.output && fn.output->ret && fn.output->ret->id == id) {
        return &fn.output->ret->typeName;
      }
      const pt::FunctionParameter* param = paramOf(fn, id);
      return param == nullptr ? nullptr : &param->typeName;
    }

    Rect headerOf(const Rect& frame, const EditorContext::Layout& layout) {
      return {frame.x, frame.y, frame.w, layout.headerH()};
    }

  }  // namespace

  TerminalSet anchors(const Signature& fn, fluir::ID id, const Rect& rect) {
    const bool isReturn = fn.output && fn.output->ret && fn.output->ret->id == id;
    const double midY = rect.y + rect.h * 0.5;
    return isReturn ? TerminalSet{{Vec2{rect.x, midY}}, {}} : TerminalSet{{}, {Vec2{rect.x + rect.w, midY}}};
  }

  Rect paramOrReturnRect(const Signature& fn, fluir::ID id, const Rect& frame, const EditorContext::Layout& layout) {
    const Vec2 origin = bodyOrigin(frame.topLeft(), layout.headerH());
    if (fn.output && fn.output->ret && fn.output->ret->id == id) {
      return {origin.x + frame.w - layout.returnInsetUnits * layout.unitPx, origin.y, layout.rowH(), layout.rowH()};
    }
    const std::optional<int> row = positionOf(fn, id);
    if (!row) {
      return {};
    }
    return {origin.x, origin.y + static_cast<double>(*row) * layout.rowH(), layout.paramW(), layout.rowH()};
  }

  Color color(const Signature&, const EditorContext::Theme& theme) { return theme.funcDeclHeader; }

  Vec2i defaultSize(const Signature&) { return DEFAULT_SIZE; }

  Limits<Vec2i> sizeLimits(const Signature&) { return SIZE_LIMITS; }

  std::vector<FieldLabel> labels(const Signature&, const Rect& frame, const EditorContext::Layout& layout) {
    return {{{Field::Kind::Name}, splitLabel(headerOf(frame, layout), FN_TAG, layout).text}};
  }

  std::vector<FieldLabel> labels(const Signature& fn,
                                 fluir::ID id,
                                 const Rect& rect,
                                 const EditorContext::Layout& layout) {
    const std::string* typeName = typeNameAt(fn, id);
    if (typeName == nullptr) {
      return {};
    }
    const SplitLabel split = splitLabel(rect, *typeName, layout);
    const std::optional<int> position = positionOf(fn, id);
    if (!position) {
      return {{{Field::Kind::ReturnType}, split.tag, FieldLook::Tag}};
    }
    return {{{Field::Kind::ParamType, *position}, split.tag, FieldLook::Tag},
            {{Field::Kind::ParamName, *position}, split.text}};
  }

}  // namespace fluir::editor::draw
