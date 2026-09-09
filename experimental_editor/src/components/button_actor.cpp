#include "editor/components/button_actor.hpp"

#include "editor/core/editor_context.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  void ButtonActor::onClick(Vec2) {
    if (action_) {
      action_();
    }
  }

  void ButtonActor::drawSelf(const Subview& body, const EditorContext& ctx) const {
    body.renderer().fillRect(body.toScreen(bounds()), ctx.theme.operatorNode);
    body.renderer().drawRect(body.toScreen(bounds()), ctx.theme.border);

    const Vec2 textPos{bounds().x + ctx.layout.textPad, bounds().y + ctx.layout.textPad};
    body.renderer().drawText(body.toScreen(textPos), label_, ctx.theme.text);
  }

}  // namespace fluir::editor
