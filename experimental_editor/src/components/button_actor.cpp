#include "editor/components/button_actor.hpp"

#include "editor/core/editor_context.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  void ButtonActor::onClick(Vec2) {
    if (opts_.onClick) {
      opts_.onClick();
    }
  }

  void ButtonActor::drawSelf(const Subview& body, const EditorContext& ctx) const {
    auto color = ctx.theme.buttonEnabled;
    if (opts_.isActive && !opts_.isActive()) {
      color = ctx.theme.buttonDisabled;
    }
    body.renderer().fillRect(body.toScreen(bounds()), color);
    body.renderer().drawRect(body.toScreen(bounds()), ctx.theme.border);

    const Vec2 textPos{bounds().x + ctx.layout.textPad, bounds().y + ctx.layout.textPad};
    body.renderer().drawText(body.toScreen(textPos), label_, ctx.theme.text);
  }

}  // namespace fluir::editor
