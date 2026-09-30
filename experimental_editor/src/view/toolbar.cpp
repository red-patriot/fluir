#include "editor/view/toolbar.hpp"

#include <ranges>

namespace fluir::editor {

  ToolbarLayout layoutToolbar(Toolbar& toolbar,
                              double width,
                              const EditorContext::Layout& layout,
                              TextMetrics& metrics) {
    const double margin = layout.textPad;
    const double buttonH = layout.chromeHeaderPx - 2 * margin;
    ToolbarLayout out{.bar = Rect{0, 0, width, layout.chromeHeaderPx}};

    double left = margin;
    for (auto& button : toolbar.left) {
      const double w = button.preferredSize(layout.textPad, metrics).x;
      button.place(Rect{left, margin, w, buttonH});
      left += w + margin;
    }
    out.labelX = left;

    double right = width - margin;
    for (auto& button : std::views::reverse(toolbar.right)) {
      const double w = button.preferredSize(layout.textPad, metrics).x;
      right -= w;
      button.place(Rect{right, margin, w, buttonH});
      right -= margin;
    }
    return out;
  }

  void drawToolbar(Renderer& renderer, const Toolbar& toolbar, const ToolbarLayout& layout, const EditorContext& ctx) {
    renderer.fillRect(layout.bar, ctx.theme.headerBackground);
    if (!toolbar.label.empty()) {
      renderer.drawText(Vec2{layout.labelX, layout.bar.y + ctx.layout.textPad}, toolbar.label, ctx.theme.text);
    }
    for (const auto& button : toolbar.left) {
      button.draw(renderer, ctx.theme);
    }
    for (const auto& button : toolbar.right) {
      button.draw(renderer, ctx.theme);
    }
  }

  bool handleToolbar(Toolbar& toolbar, const InputEvent& event) {
    for (auto& button : toolbar.left) {
      if (button.handle(event)) {
        return true;
      }
    }
    for (auto& button : toolbar.right) {
      if (button.handle(event)) {
        return true;
      }
    }
    return false;
  }

}  // namespace fluir::editor
