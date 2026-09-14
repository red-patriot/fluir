#include "editor/tools/menu_popup.hpp"

#include <utility>

namespace fluir::editor {

  MenuPopup::MenuPopup(
    std::vector<std::string> labels, Rect anchor, Rect bounds, const EditorContext::Layout& layout, OnPick onPick) :
    labels_(std::move(labels)), layout_(layoutMenu(labels_, anchor, bounds, layout)), onPick_(std::move(onPick)) { }

  bool MenuPopup::onEvent(const InputEvent& event, EditorState& state) {
    switch (event.type) {
      case InputEvent::Type::MouseMove:
        hovered_ = menuItemAt(layout_, event.pos);
        return true;
      case InputEvent::Type::MouseDown:
        {
          const std::optional<std::size_t> item = menuItemAt(layout_, event.pos);
          if (item && event.button == InputEvent::Button::Left) {
            onPick_(*item, state);
            return false;
          }
          return layout_.frame.contains(event.pos);
        }
      case InputEvent::Type::KeyDown:
        return event.key != InputEvent::Key::Escape;
      default:
        return true;
    }
  }

  void MenuPopup::draw(Renderer& renderer, const EditorContext& ctx) const {
    drawMenu(renderer, labels_, layout_, hovered_, ctx);
  }

}  // namespace fluir::editor
