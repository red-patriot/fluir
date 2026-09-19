#include "editor/view/draw/call.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "editor/core/renderer.hpp"

namespace fluir::editor::draw {
  namespace {

    std::vector<const pt::Call::Argument*> sortedArgs(const pt::Call& call) {
      std::vector<const pt::Call::Argument*> args;
      for (const auto& arg : call.arguments) {
        args.push_back(&arg);
      }
      std::ranges::sort(args, {}, &pt::Call::Argument::index);
      return args;
    }

    // One row per argument below the header row, each input at its row's centre.
    double argRowTop(const Rect& rect, std::size_t row, const EditorContext::Layout& layout) {
      return rect.y + (static_cast<double>(row) + 1.0) * layout.railStep();
    }

  }  // namespace

  PortSet anchors(const pt::Call& call, const Rect& r, const EditorContext::Layout& layout) {
    PortSet out;
    const std::size_t count = call.arguments.size();
    for (std::size_t row = 0; row < count; ++row) {
      out.inputs.push_back(Vec2{r.x, argRowTop(r, row, layout) + layout.railStep() * 0.5});
    }
    if (call._return) {
      out.outputs = edgeAnchors(r.x + r.w, r, 1);
    }
    return out;
  }

  Color color(const pt::Call&, const EditorContext::Theme& theme) { return theme.callNode; }

  void draw(const pt::Call& call, const Rect& world, const Subview& view, const EditorContext& ctx) {
    drawShell(world, color(call, ctx.theme), view, ctx);
    drawTitle(call.target, world, view, ctx);
    const std::vector<const pt::Call::Argument*> args = sortedArgs(call);
    for (std::size_t row = 0; row < args.size(); ++row) {
      const Vec2 pos{world.x + ctx.layout.textPad, argRowTop(world, row, ctx.layout) + ctx.layout.textPad};
      view.renderer().drawText(view.toScreen(pos), args[row]->name, ctx.theme.text, view.composed().scale);
    }
    drawPortDots(anchors(call, world, ctx.layout), view, ctx);
  }

}  // namespace fluir::editor::draw
