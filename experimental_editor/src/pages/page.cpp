#include "editor/pages/page.hpp"

namespace fluir::editor {

  Rect Page::outputRect() const {
    const Vec2 size = renderer_.outputSize();
    return Rect{0, 0, size.x, size.y};
  }

  int Page::start() {
    const int status = onStart();
    if (status != 0) {
      return status;
    }
    onResize();
    return 0;
  }

  int Page::update(const std::vector<InputEvent>& events) {
    for (const InputEvent& event : events) {
      if (event.type == InputEvent::Type::Quit) {
        ctx_.running = false;
      } else if (event.type == InputEvent::Type::Resize) {
        onResize();
      } else {
        onEvent(event);
      }
    }
    return 0;
  }

  int Page::draw() {
    renderer_.beginFrame();
    onDraw();
    renderer_.endFrame();
    return 0;
  }

}  // namespace fluir::editor
