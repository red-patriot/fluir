#include "editor/tools/context_menu_tool.hpp"

#include <cstddef>
#include <memory>
#include <utility>

#include "editor/tools/menu_popup.hpp"

namespace fluir::editor {

  ContextMenuTool::ContextMenuTool(std::vector<MenuProvider> providers) : providers_(std::move(providers)) { }

  bool ContextMenuTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    if (event.type != InputEvent::Type::MouseDown || event.button != InputEvent::Button::Right) {
      return false;
    }
    const Vec2 world = state.view.screenToWorld(event.pos);
    const Box* hit = hitAt(boxes, world);
    if (hit == nullptr) {
      return false;  // the background belongs to CompletionTool
    }
    for (const MenuProvider& provider : providers_) {
      std::vector<MenuItem> items = provider(*hit, world, state);
      if (items.empty()) {
        continue;
      }
      std::vector<std::string> labels;
      std::vector<bool> enabled;
      for (const MenuItem& item : items) {
        labels.push_back(item.label);
        enabled.push_back(item.enabled);
      }
      state.popup = std::make_unique<MenuPopup>(
        std::move(labels),
        Rect{event.pos.x, event.pos.y, 0, 0},
        popupBounds(state),
        state.ctx.layout,
        state.text,
        [items = std::move(items)](std::size_t i, EditorState& s) { items[i].onClick(s); },
        std::move(enabled));
      return true;  // consumed so no later tool replaces the menu
    }
    return false;
  }

}  // namespace fluir::editor
