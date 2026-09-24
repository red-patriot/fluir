#include "editor/tools/choice_tool.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "editor/core/fields.hpp"
#include "editor/tools/menu_popup.hpp"

namespace fluir::editor {

  bool ChoiceTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    if (event.type != InputEvent::Type::MouseDown || event.button != InputEvent::Button::Left) {
      return false;
    }
    const Box* label = labelAt(boxes, state.view.screenToWorld(event.pos));
    if (label == nullptr || !isChoice(label->field->kind)) {
      return false;
    }
    std::vector<std::string> choices = state.intelligence.choices(state.editor.tree(), label->path, *label->field);
    if (choices.empty()) {
      return false;
    }
    state.popup = std::make_unique<MenuPopup>(
      choices,
      state.view.toScreen(label->world),
      popupBounds(state),
      state.ctx.layout,
      state.text,
      [path = label->path, field = *label->field, choices](std::size_t i, EditorState& s) {
        std::optional<std::unique_ptr<Transaction>> edit = fields::write(s.editor.tree(), path, field, choices[i]);
        if (edit && *edit) {
          s.editor.apply(std::move(*edit));
        }
      });
    return false;  // tracked, never consumed
  }

}  // namespace fluir::editor
