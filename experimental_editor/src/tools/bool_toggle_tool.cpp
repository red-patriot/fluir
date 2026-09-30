#include "editor/tools/bool_toggle_tool.hpp"

#include <memory>
#include <variant>

#include "compiler/models/literal_types.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/set_constant_value.hpp"

namespace fluir::editor {
  namespace {

    const literals_types::BOOL* boolAt(const et::ParseTree& tree, const FullID& path) {
      const et::Node* node = nodeAt(tree, path);
      const auto* constant = node == nullptr ? nullptr : std::get_if<et::Constant>(node);
      return constant == nullptr ? nullptr : std::get_if<literals_types::BOOL>(&constant->value);
    }

  }  // namespace

  bool BoolToggleTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    if (event.type != InputEvent::Type::MouseDown || event.button != InputEvent::Button::Left) {
      return false;
    }
    const Box* label = labelAt(boxes, state.view.screenToWorld(event.pos));
    if (label == nullptr || label->field->kind != Field::Kind::Bool) {
      return false;
    }
    const literals_types::BOOL* value = boolAt(state.editor.tree(), label->path);
    if (value == nullptr) {
      return false;
    }
    state.editor.apply(setConstantValue(label->path, et::Literal{!*value}));
    return true;
  }

}  // namespace fluir::editor
