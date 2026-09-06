#include "editor/core/layer.hpp"

namespace fluir::editor {

  void Layer::draw(Renderer& renderer, const EditorContext& ctx, Rect outputRect) const {
    const Subview root{viewport_, outputRect, renderer};
    for (Actor* actor : actors_) {
      actor->draw(root, ctx);
    }
  }

  Actor* Layer::topmostAt(Vec2 screenPos) const {
    const Vec2 local = viewport_.screenToWorld(screenPos);
    for (auto it = actors_.rbegin(); it != actors_.rend(); ++it) {
      if ((*it)->bounds().contains(local)) {
        return *it;
      }
    }
    return nullptr;
  }

  bool Layer::handleEvent(const InputEvent& event) {
    if (event.type != InputEvent::Type::MouseDown || event.button != InputEvent::Button::Left) {
      return false;
    }
    if (auto* hit = topmostAt(event.pos); hit) {
      hit->onClick(viewport_.screenToWorld(event.pos));
      return true;
    }
    return false;
  }

}  // namespace fluir::editor
