#include "editor/tools/operator_tool.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "compiler/models/operator.hpp"
#include "editor/tools/menu_popup.hpp"
#include "editor/transaction/edit_operator.hpp"

namespace fluir::editor {

  bool OperatorTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    if (event.type != InputEvent::Type::MouseDown || event.button != InputEvent::Button::Left) {
      return false;
    }
    const Box* hit = hitAt(boxes, state.view.screenToWorld(event.pos));
    if (hit == nullptr || hit->part != Part::Body) {
      return false;
    }
    std::vector<fluir::Operator> ops = state.intelligence.operators(state.editor.tree(), hit->path);
    if (ops.empty()) {
      return false;
    }
    std::vector<std::string> labels;
    for (const fluir::Operator op : ops) {
      labels.emplace_back(stringify(op));
    }
    const Vec2 topLeft = state.view.worldToScreen(hit->world.topLeft());
    const Rect anchor{topLeft.x, topLeft.y, hit->world.w * state.view.scale, hit->world.h * state.view.scale};
    state.popup = std::make_unique<MenuPopup>(std::move(labels),
                                              anchor,
                                              popupBounds(state),
                                              state.ctx.layout,
                                              state.text,
                                              [path = hit->path, ops = std::move(ops)](std::size_t i, EditorState& s) {
                                                s.editor.apply(std::make_unique<EditOperatorTransaction>(path, ops[i]));
                                              });
    return false;  // tracked, never consumed
  }

}  // namespace fluir::editor
