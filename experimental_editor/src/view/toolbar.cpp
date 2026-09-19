#include "editor/view/toolbar.hpp"

namespace fluir::editor {
  namespace {

    double buttonWidth(const Button& button, const EditorContext::Layout& layout, Renderer& renderer) {
      return renderer.measureText(button.label).x + 2 * layout.textPad;
    }

  }  // namespace

  ToolbarLayout layoutToolbar(const Toolbar& toolbar,
                              double width,
                              const EditorContext::Layout& layout,
                              Renderer& renderer) {
    const double margin = layout.textPad;
    const double buttonH = layout.chromeHeaderPx - 2 * margin;
    ToolbarLayout out{.bar = Rect{0, 0, width, layout.chromeHeaderPx},
                      .buttons = std::vector<Rect>(toolbar.buttons.size())};

    double left = margin;
    for (std::size_t i = 0; i < toolbar.buttons.size(); ++i) {
      if (toolbar.buttons[i].align == Button::Align::Left) {
        const double w = buttonWidth(toolbar.buttons[i], layout, renderer);
        out.buttons[i] = Rect{left, margin, w, buttonH};
        left += w + margin;
      }
    }
    out.labelX = left;

    double right = width - margin;
    for (std::size_t i = toolbar.buttons.size(); i-- > 0;) {
      if (toolbar.buttons[i].align == Button::Align::Right) {
        const double w = buttonWidth(toolbar.buttons[i], layout, renderer);
        right -= w;
        out.buttons[i] = Rect{right, margin, w, buttonH};
        right -= margin;
      }
    }
    return out;
  }

  void drawToolbar(Renderer& renderer, const Toolbar& toolbar, const ToolbarLayout& layout, const EditorContext& ctx) {
    renderer.fillRect(layout.bar, ctx.theme.headerBackground);
    if (!toolbar.label.empty()) {
      renderer.drawText(Vec2{layout.labelX, layout.bar.y + ctx.layout.textPad}, toolbar.label, ctx.theme.text);
    }
    for (std::size_t i = 0; i < toolbar.buttons.size() && i < layout.buttons.size(); ++i) {
      drawButton(renderer, toolbar.buttons[i], layout.buttons[i], ctx);
    }
  }

  void drawButton(Renderer& renderer, const Button& button, Rect rect, const EditorContext& ctx) {
    const bool enabled = !button.enabled || button.enabled();
    renderer.fillRect(rect, enabled ? ctx.theme.buttonEnabled : ctx.theme.buttonDisabled);
    renderer.drawRect(rect, ctx.theme.border);
    renderer.drawText(Vec2{rect.x + ctx.layout.textPad, rect.y + ctx.layout.textPad}, button.label, ctx.theme.text);
  }

  std::optional<std::size_t> buttonAt(const ToolbarLayout& layout, Vec2 screen) {
    for (std::size_t i = 0; i < layout.buttons.size(); ++i) {
      if (layout.buttons[i].contains(screen)) {
        return i;
      }
    }
    return std::nullopt;
  }

}  // namespace fluir::editor
