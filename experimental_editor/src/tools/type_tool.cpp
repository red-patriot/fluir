#include "editor/tools/type_tool.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "editor/core/tree_path.hpp"
#include "editor/tools/menu_popup.hpp"
#include "editor/transaction/update_func_param.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor {
  namespace {

    // The rail whose type tag holds `world`, or nullptr.
    const Box* railTagAt(const EditorState& state, std::span<const Box> boxes, Vec2 world) {
      const Box* rail = hitAt(boxes, world);
      const pt::FunctionDecl* fn =
        !rail || rail->part != Part::Rail ? nullptr : functionAt(state.editor.tree(), parentOf(rail->path));
      const std::string* type = !fn ? nullptr : railTypeAt(*fn, rail->path.back());
      return type && splitLabel(rail->world, *type, state.ctx.layout).tag.contains(world) ? rail : nullptr;
    }

  }  // namespace

  bool TypeTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    if (event.type != InputEvent::Type::MouseDown || event.button != InputEvent::Button::Left) {
      return false;
    }
    const Box* rail = railTagAt(state, boxes, state.view.screenToWorld(event.pos));
    if (rail == nullptr) {
      return false;
    }
    const std::vector<std::string_view> types = state.intelligence.types(state.editor.tree(), rail->path);
    if (types.empty()) {
      return false;
    }
    std::vector<std::string> labels(types.begin(), types.end());
    const Rect anchor = state.view.toScreen(rail->world);
    state.popup = std::make_unique<MenuPopup>(
      labels,
      anchor,
      popupBounds(state),
      state.ctx.layout,
      state.text,
      [fnPath = parentOf(rail->path), id = rail->path.back(), labels](std::size_t i, EditorState& s) {
        s.editor.apply(setRailType(fnPath, id, labels[i]));
      });
    return false;  // tracked, never consumed
  }

}  // namespace fluir::editor
