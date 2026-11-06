#include "editor/view/draw/call.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace fluir::editor::draw {
  namespace {

    // In grid units. A node's height follows its content, so it is unbounded below.
    constexpr Limits<Vec2i> SIZE_LIMITS{.lower = Vec2i{4, 0}, .upper = Vec2i{1000, 1000}};
    constexpr int DEFAULT_W = 14;
    constexpr int ROW_H = 5;

    // One row per argument below the header row, each input at its row's centre.
    double argRowTop(const Rect& rect, std::size_t row, const EditorContext::Layout& layout) {
      return rect.y + (static_cast<double>(row) + 1.0) * layout.rowH();
    }

    // `call`'s arguments, ascending by index.
    std::vector<const pt::Call::Argument*> sortedArguments(const pt::Call& call) {
      std::vector<const pt::Call::Argument*> out;
      for (const auto& arg : call.arguments) {
        out.push_back(&arg);
      }
      std::ranges::sort(out, {}, &pt::Call::Argument::index);
      return out;
    }

  }  // namespace

  TerminalSet anchors(const pt::Call& call, const Rect& r, const EditorContext::Layout& layout) {
    TerminalSet out;
    const std::size_t count = call.arguments.size();
    for (std::size_t row = 0; row < count; ++row) {
      out.inputs.push_back(Vec2{r.x, argRowTop(r, row, layout) + layout.rowH() * 0.5});
    }
    if (call._return) {
      out.outputs = edgeAnchors(r.x + r.w, r, 1);
    }
    return out;
  }

  Color color(const pt::Call&, const EditorContext::Theme& theme) { return theme.callNode; }

  Limits<Vec2i> sizeLimits(const pt::Call&) { return SIZE_LIMITS; }

  Vec2i defaultSize(const pt::Call& node) { return {DEFAULT_W, ROW_H * (static_cast<int>(node.arguments.size()) + 1)}; }

  std::optional<ResizeAxes> resizePart(const pt::Call&) { return ResizeAxes::X; }

  std::vector<FieldLabel> labels(const pt::Call& call, const Rect& r, const EditorContext::Layout& layout) {
    std::vector<FieldLabel> out{{{Field::Kind::Target}, {r.x, r.y, r.w, std::min(r.h, layout.rowH())}}};
    const std::vector<const pt::Call::Argument*> args = sortedArguments(call);
    for (std::size_t row = 0; row < args.size(); ++row) {
      out.push_back({{Field::Kind::Arg, args[row]->index}, {r.x, argRowTop(r, row, layout), r.w, layout.rowH()}});
    }
    return out;
  }

}  // namespace fluir::editor::draw
