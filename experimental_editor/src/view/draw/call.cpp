#include "editor/view/draw/call.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "editor/core/node_access.hpp"

namespace fluir::editor::draw {
  namespace {

    // In grid units. A node's height follows its content, so it is unbounded below.
    constexpr Limits<Vec2i> SIZE_LIMITS{.lower = Vec2i{4, 0}, .upper = Vec2i{1000, 1000}};

    // One row per argument below the header row, each input at its row's centre.
    double argRowTop(const Rect& rect, std::size_t row, const EditorContext::Layout& layout) {
      return rect.y + (static_cast<double>(row) + 1.0) * layout.railStep();
    }

  }  // namespace

  TerminalSet anchors(const et::Call& call, const Rect& r, const EditorContext::Layout& layout) {
    TerminalSet out;
    const std::size_t count = call.arguments.size();
    for (std::size_t row = 0; row < count; ++row) {
      out.inputs.push_back(Vec2{r.x, argRowTop(r, row, layout) + layout.railStep() * 0.5});
    }
    if (call._return) {
      out.outputs = edgeAnchors(r.x + r.w, r, 1);
    }
    return out;
  }

  Color color(const et::Call&, const EditorContext::Theme& theme) { return theme.callNode; }

  Limits<Vec2i> sizeLimits(const et::Call&) { return SIZE_LIMITS; }

  std::optional<Part> resizePart(const et::Call&) { return Part::ResizeX; }

  std::vector<FieldLabel> labels(const et::Call& call, const Rect& r, const EditorContext::Layout& layout) {
    std::vector<FieldLabel> out{{{Field::Kind::Target}, {r.x, r.y, r.w, std::min(r.h, layout.railStep())}}};
    const std::vector<const et::Call::Argument*> args = sortedArguments(call);
    for (std::size_t row = 0; row < args.size(); ++row) {
      out.push_back({{Field::Kind::Arg, args[row]->index}, {r.x, argRowTop(r, row, layout), r.w, layout.railStep()}});
    }
    return out;
  }

  void draw(const et::Call& call, const Rect& world, const Subview& view, const EditorContext& ctx) {
    drawShell(world, color(call, ctx.theme), view, ctx);
    // Rows follow `labels`' order: the target, then each argument by index.
    const std::vector<FieldLabel> rows = labels(call, world, ctx.layout);
    const std::vector<const et::Call::Argument*> args = sortedArguments(call);
    drawTitle(call.target, rows.front().rect, view, ctx);
    for (std::size_t row = 0; row < args.size(); ++row) {
      drawTitle(args[row]->name, rows[row + 1].rect, view, ctx);
    }
    drawTerminalDots(anchors(call, world, ctx.layout), view, ctx);
  }

}  // namespace fluir::editor::draw
