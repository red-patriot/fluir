#include "editor/pages/header_bar.hpp"

#include <utility>

#include "editor/core/viewport.hpp"

namespace fluir::editor {

  HeaderBar::HeaderBar(std::function<void()> onExit) : exitButton_("Exit", std::move(onExit), Rect{0, 0, 0, 0}) { }

  void HeaderBar::draw(const EditorContext& ctx, Renderer& renderer, Vec2 outputSize) {
    const Viewport identity;
    const Rect barRect{0, 0, outputSize.x, ctx.layout.chromeHeaderPx};
    const Subview bar{identity, barRect, renderer};

    bar.renderer().fillRect(bar.toScreen(barRect), ctx.theme.headerBackground);

    if (ctx.program.has_value()) {
      const Vec2 textPos{ctx.layout.textPad, ctx.layout.textPad};
      bar.renderer().drawText(bar.toScreen(textPos), ctx.program->filename().string(), ctx.theme.text);
    }

    const double margin = ctx.layout.textPad;
    const double buttonW = 60.0;
    const double buttonH = ctx.layout.chromeHeaderPx - 2 * margin;
    exitButton_.setBounds(Rect{outputSize.x - margin - buttonW, margin, buttonW, buttonH});
    exitButton_.draw(bar, ctx);
  }

  bool HeaderBar::handleClick(Vec2 screenPos) {
    if (exitButton_.bounds().contains(screenPos)) {
      exitButton_.onClick(screenPos);
      return true;
    }
    return false;
  }

}  // namespace fluir::editor
