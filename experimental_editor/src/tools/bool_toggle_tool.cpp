#include "editor/tools/bool_toggle_tool.hpp"

#include <memory>
#include <variant>

#include "compiler/models/literal_types.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/set_constant_value.hpp"
#include "editor/view/draw/constant.hpp"

namespace fluir::editor {
  namespace {

    const literals_types::BOOL* boolAt(const pt::ParseTree& tree, const FullID& path) {
      const pt::Node* node = nodeAt(tree, path);
      const auto* constant = node == nullptr ? nullptr : std::get_if<pt::Constant>(node);
      return constant == nullptr ? nullptr : std::get_if<literals_types::BOOL>(&constant->value);
    }

  }  // namespace

  bool BoolToggleTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    if (event.type != InputEvent::Type::MouseDown || event.button != InputEvent::Button::Left) {
      return false;
    }
    const Vec2 world = state.view.screenToWorld(event.pos);
    const Box* hit = hitAt(boxes, world);
    if (hit == nullptr || hit->part != Part::Body) {
      return false;
    }
    const literals_types::BOOL* value = boolAt(state.editor.tree(), hit->path);
    if (value == nullptr || !draw::boolToggleRect(hit->world, state.ctx.layout).contains(world)) {
      return false;
    }
    state.editor.apply(std::make_unique<SetConstantValueTransaction>(hit->path, pt::Literal{!*value}));
    return true;
  }

}  // namespace fluir::editor
