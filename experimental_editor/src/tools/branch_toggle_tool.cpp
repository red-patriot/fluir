#include "editor/tools/branch_toggle_tool.hpp"

#include <variant>

#include "editor/core/tree_path.hpp"
#include "editor/view/draw/conditional.hpp"

namespace fluir::editor {

  bool BranchToggleTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    if (event.type != InputEvent::Type::MouseDown || event.button != InputEvent::Button::Left) {
      return false;
    }
    const Vec2 world = state.view.screenToWorld(event.pos);
    const Box* hit = hitAt(boxes, world);
    if (hit == nullptr || hit->part != Part::Header) {
      return false;
    }
    auto* conditional = std::get_if<et::Conditional>(nodeAt(state.editor.tree(), hit->path));
    if (conditional == nullptr || !draw::branchArrowRect(hit->world, state.ctx.layout).contains(world)) {
      return false;
    }
    fluir::ID& shown = conditional->annotation.shownBranch;
    shown = shown == ELSE_BRANCH_ID ? THEN_BRANCH_ID : ELSE_BRANCH_ID;
    return true;
  }

}  // namespace fluir::editor
