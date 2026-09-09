#include "editor/components/toolbar_actor.hpp"

#include <memory>
#include <utility>

#include "editor/core/viewport.hpp"

namespace fluir::editor {

  ButtonActor& ToolbarActor::add(std::string label, std::function<void()> action, Align align) {
    auto& button = static_cast<ButtonActor&>(
      Actor::add(std::make_unique<ButtonActor>(std::move(label), std::move(action), Rect{0, 0, 0, 0})));
    entries_.push_back(Entry{&button, align});
    return button;
  }

  double ToolbarActor::buttonWidth(const EditorContext& ctx, const ButtonActor& button) const {
    return renderer_.measureText(button.label()).x + 2 * ctx.layout.textPad;
  }

  void ToolbarActor::resize(const EditorContext& ctx, double width) {
    setBounds(Rect{bounds().x, bounds().y, width, ctx.layout.chromeHeaderPx});
    layout(ctx);
  }

  void ToolbarActor::layout(const EditorContext& ctx) {
    const double margin = ctx.layout.textPad;
    const double buttonH = ctx.layout.chromeHeaderPx - 2 * margin;

    double left = margin;
    for (const Entry& entry : entries_) {
      if (entry.align != Align::Left) {
        continue;
      }
      const double w = buttonWidth(ctx, *entry.button);
      entry.button->setBounds(Rect{left, margin, w, buttonH});
      left += w + margin;
    }
    labelX_ = left;

    double right = bounds().w - margin;
    for (auto it = entries_.rbegin(); it != entries_.rend(); ++it) {
      if (it->align != Align::Right) {
        continue;
      }
      const double w = buttonWidth(ctx, *it->button);
      right -= w;
      it->button->setBounds(Rect{right, margin, w, buttonH});
      right -= margin;
    }

    Actor::layout(ctx);
  }

  void ToolbarActor::drawSelf(const Subview& view, const EditorContext& ctx) const {
    view.renderer().fillRect(view.toScreen(bounds()), ctx.theme.headerBackground);
    if (!label_.empty()) {
      view.renderer().drawText(
        view.toScreen(Vec2{bounds().x + labelX_, bounds().y + ctx.layout.textPad}), label_, ctx.theme.text);
    }
  }

}  // namespace fluir::editor
