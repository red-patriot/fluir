#include "editor/components/button.hpp"

#include <utility>

namespace fluir::editor {
  Button::Button(ButtonOptions options) : options_(std::move(options)) { }

  Vec2 Button::preferredSize(double textPad, TextMetrics& metrics) const {
    const auto textSize = metrics.measureText(options_.label);

    return Vec2{.x = textSize.x + textPad * 2, .y = textSize.y + textPad * 2};
  }

  bool Button::enabled() const { return !options_.enabled || options_.enabled(); }

  bool Button::handle(const InputEvent& event) {
    if (!event.button) {
      return false;
    }
    if (!rect_.contains(event.pos)) {
      const auto wasArmed = armed_;
      armed_ = false;
      return wasArmed;
    }
    if (*event.button != InputEvent::Button::Left) {
      armed_ = false;
      return false;
    }

    if (event.type == InputEvent::Type::MouseDown && !armed_) {
      armed_ = true;
      return true;
    }
    if (event.type == InputEvent::Type::MouseUp && armed_) {
      armed_ = false;
      if (options_.onClick && enabled()) {
        options_.onClick();
      }
      return true;
    }
    armed_ = false;
    return false;
  }

  void Button::draw(Renderer& renderer, const EditorContext::Theme& theme) const {
    if (rect_.w == 0 || rect_.h == 0) {
      return;
    }
    renderer.fillRect(rect_, enabled() ? theme.buttonEnabled : theme.buttonDisabled);
    drawCenteredLabel(renderer, theme);
    renderer.drawRect(rect_, theme.border);
  }

  void Button::drawCenteredLabel(Renderer& renderer, const EditorContext::Theme& theme) const {
    const auto textSize = renderer.measureText(options_.label);
    const auto center = rect_.center();
    const auto topLeft = center - textSize / 2;
    renderer.drawText(topLeft, options_.label, theme.text);
  }
}  // namespace fluir::editor
