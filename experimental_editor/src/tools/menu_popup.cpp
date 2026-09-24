#include "editor/tools/menu_popup.hpp"

#include <limits>
#include <utility>

#include "editor/tools/tool.hpp"

namespace fluir::editor {

  Rect popupBounds(const EditorState& state) {
    if (!state.screen) {
      const double unbounded = std::numeric_limits<double>::max() / 4;
      return Rect{-unbounded, -unbounded, 2 * unbounded, 2 * unbounded};
    }
    return *state.screen;
  }

  MenuPopup::MenuPopup(std::vector<std::string> labels,
                       Rect anchor,
                       Rect bounds,
                       const EditorContext::Layout& layout,
                       TextMetrics* text,
                       OnPick onPick,
                       std::vector<bool> enabled) :
    labels_(std::move(labels)),
    layout_(layoutMenu(labels_, anchor, bounds, layout, text)),
    onPick_(std::move(onPick)),
    enabled_(std::move(enabled)) { }

  bool MenuPopup::onEvent(const InputEvent& event, EditorState& state) {
    switch (event.type) {
      case InputEvent::Type::MouseMove:
        hovered_ = menuItemAt(layout_, event.pos);
        if (hovered_ && !menuRowEnabled(enabled_, *hovered_)) {
          hovered_.reset();
        }
        return true;
      case InputEvent::Type::MouseDown:
        {
          const std::optional<std::size_t> item = menuItemAt(layout_, event.pos);
          if (item && event.button == InputEvent::Button::Left) {
            if (!menuRowEnabled(enabled_, *item)) {
              return true;
            }
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
    drawMenu(renderer, labels_, layout_, hovered_, ctx, enabled_);
  }

}  // namespace fluir::editor
