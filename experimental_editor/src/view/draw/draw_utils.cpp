#include "editor/view/draw/draw_utils.hpp"

#include <algorithm>

#include "editor/core/graph_geometry.hpp"
#include "editor/core/renderer.hpp"

namespace fluir::editor {
  namespace {

    // Split-label tags draw smaller than their text.
    constexpr double kTagScale = 0.8;

    // A lone port sits at the centre; more spread top-to-bottom down the edge.
    double portFraction(int count, int index) {
      return count <= 1 ? 0.5 : static_cast<double>(index) / (static_cast<double>(count) - 1.0);
    }

  }  // namespace

  SplitLabel splitLabel(Rect box, std::string_view tag, double viewScale, const EditorContext::Layout& layout) {
    const double tagW = layout.textPad + static_cast<double>(tag.size()) * GLYPH_PX * kTagScale / viewScale;
    return {Rect{box.x, box.y, tagW, box.h}, Rect{box.x + tagW, box.y, std::max(0.0, box.w - tagW), box.h}};
  }

  void drawSplitLabel(
    const Subview& view, Rect box, std::string_view tag, std::string_view text, const EditorContext& ctx) {
    Renderer& r = view.renderer();
    const double pad = ctx.layout.textPad;
    const Vec2 tagBottom = view.toScreen(Vec2{box.x + pad, box.y + box.h - pad});
    r.drawText(Vec2{tagBottom.x, tagBottom.y - r.measureText(tag).y * kTagScale}, tag, ctx.theme.text, kTagScale);
    if (!text.empty()) {
      const Rect textRect = splitLabel(box, tag, view.composed().scale, ctx.layout).text;
      r.drawText(view.toScreen(Vec2{textRect.x + pad, box.y + pad}), text, ctx.theme.text);
    }
  }

  namespace draw {

    std::vector<Vec2> edgeAnchors(double edgeX, const Rect& rect, int count) {
      std::vector<Vec2> out;
      for (int i = 0; i < count; ++i) {
        out.push_back(Vec2{edgeX, rect.y + portFraction(count, i) * rect.h});
      }
      return out;
    }

    void drawShell(const Rect& world, Color fill, const Subview& view, const EditorContext& ctx) {
      view.renderer().fillRect(view.toScreen(world), fill);
      view.renderer().drawRect(view.toScreen(world), ctx.theme.border);
    }

    void drawPortDots(const PortSet& portSet, const Subview& view, const EditorContext& ctx) {
      for (const auto* side : {&portSet.inputs, &portSet.outputs}) {
        for (const Vec2& anchor : *side) {
          view.renderer().fillRect(view.toScreen(dotRect(anchor, ctx.layout.portDot)), ctx.theme.border);
        }
      }
    }

    void drawTitle(std::string_view text, const Rect& world, const Subview& view, const EditorContext& ctx) {
      if (!text.empty()) {
        const Vec2 textPos{world.x + ctx.layout.textPad, world.y + ctx.layout.textPad};
        view.renderer().drawText(view.toScreen(textPos), text, ctx.theme.text);
      }
    }

  }  // namespace draw

}  // namespace fluir::editor
