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
    const std::vector<Layer*> ordered = layers();
    for (const InputEvent& event : events) {
      if (event.type == InputEvent::Type::Quit) {
        ctx_.running = false;
        continue;
      }
      if (event.type == InputEvent::Type::Resize) {
        onResize();
        continue;
      }
      if (onAppEvent(event)) {
        continue;
      }
      for (Layer* layer : ordered) {
        if (layer->dispatch(event, ctx_, renderer_.outputSize())) {
          break;
        }
      }
    }
    afterUpdate();
    return 0;
  }

  int Page::draw() {
    renderer_.beginFrame();
    const std::vector<Layer*> ordered = layers();
    for (auto it = ordered.rbegin(); it != ordered.rend(); ++it) {
      (*it)->draw(renderer_, ctx_, outputRect());
    }
    renderer_.endFrame();
    return 0;
  }

}  // namespace fluir::editor
