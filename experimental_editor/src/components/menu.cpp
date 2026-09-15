#include "editor/components/menu.hpp"

#include <algorithm>

namespace fluir::editor {
  namespace {

    constexpr double kRowPx = 24.0;

  }  // namespace

  MenuLayout layoutMenu(std::span<const std::string> labels,
                        Rect anchor,
                        Rect bounds,
                        const EditorContext::Layout& layout,
                        Renderer* text) {
    Vec2 largest;
    for (const std::string& label : labels) {
      const Vec2 size =
        text != nullptr ? text->measureText(label) : Vec2{static_cast<double>(label.size()) * GLYPH_PX, GLYPH_PX};
      largest = Vec2{std::max(largest.x, size.x), std::max(largest.y, size.y)};
    }
    const double w = std::max(anchor.w, largest.x + 2 * layout.textPad);
    const double rowH = std::max(kRowPx, largest.y + 2 * layout.textPad);
    const double h = static_cast<double>(labels.size()) * rowH;

    double y = anchor.y + anchor.h;
    if (y + h > bounds.y + bounds.h) {
      y = anchor.y - h;
    }
    // Left and top edges win when the menu is larger than the bounds.
    const double x = std::max(bounds.x, std::min(anchor.x, bounds.x + bounds.w - w));
    y = std::max(bounds.y, std::min(y, bounds.y + bounds.h - h));

    MenuLayout out{.frame = Rect{x, y, w, h}, .items = {}};
    for (std::size_t i = 0; i < labels.size(); ++i) {
      out.items.push_back(Rect{x, y + static_cast<double>(i) * rowH, w, rowH});
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
      const double textH = renderer.measureText(labels[i]).y;
      renderer.drawText(Vec2{row.x + ctx.layout.textPad, row.y + (row.h - textH) / 2}, labels[i], ctx.theme.text);
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
