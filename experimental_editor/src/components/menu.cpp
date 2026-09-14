#include "editor/components/menu.hpp"

#include <algorithm>

namespace fluir::editor {
  namespace {

    constexpr double kRowPx = 24.0;

  }  // namespace

  MenuLayout layoutMenu(std::span<const std::string> labels,
                        Rect anchor,
                        Rect bounds,
                        const EditorContext::Layout& layout) {
    std::size_t longest = 0;
    for (const std::string& label : labels) {
      longest = std::max(longest, label.size());
    }
    const double w = std::max(anchor.w, static_cast<double>(longest) * GLYPH_PX + 2 * layout.textPad);
    const double h = static_cast<double>(labels.size()) * kRowPx;

    double y = anchor.y + anchor.h;
    if (y + h > bounds.y + bounds.h) {
      y = anchor.y - h;
    }
    // Left edge wins when the menu is wider than the bounds.
    const double x = std::max(bounds.x, std::min(anchor.x, bounds.x + bounds.w - w));

    MenuLayout out{.frame = Rect{x, y, w, h}, .items = {}};
    for (std::size_t i = 0; i < labels.size(); ++i) {
      out.items.push_back(Rect{x, y + static_cast<double>(i) * kRowPx, w, kRowPx});
    }
    return out;
  }

  void drawMenu(Renderer& renderer,
                std::span<const std::string> labels,
                const MenuLayout& layout,
                std::optional<std::size_t> hovered,
                const EditorContext& ctx) {
    renderer.fillRect(layout.frame, ctx.theme.headerBackground);
    for (std::size_t i = 0; i < labels.size() && i < layout.items.size(); ++i) {
      const Rect& row = layout.items[i];
      if (hovered == i) {
        renderer.fillRect(row, ctx.theme.buttonEnabled);
      }
      renderer.drawText(Vec2{row.x + ctx.layout.textPad, row.y + ctx.layout.textPad}, labels[i], ctx.theme.text);
    }
    renderer.drawRect(layout.frame, ctx.theme.border);
  }

  std::optional<std::size_t> menuItemAt(const MenuLayout& layout, Vec2 screen) {
    for (std::size_t i = 0; i < layout.items.size(); ++i) {
      if (layout.items[i].contains(screen)) {
        return i;
      }
    }
    return std::nullopt;
  }

}  // namespace fluir::editor
