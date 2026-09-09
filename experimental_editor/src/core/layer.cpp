#include "editor/core/layer.hpp"

namespace fluir::editor {

  void Layer::draw(Renderer& renderer, const EditorContext& ctx, Rect outputRect) const {
    if (root_ == nullptr) {
      return;
    }
    const Subview root{viewport_, outputRect, renderer};
    root_->draw(root, ctx);
  }

  Actor* Layer::topmostAt(Vec2 screenPos) const {
    return root_ == nullptr ? nullptr : root_->hitTest(viewport_.screenToWorld(screenPos));
  }

  bool Layer::dispatch(const InputEvent& event, EditorContext& ctx, Vec2 outputSize) {
    if (root_ == nullptr) {
      return false;
    }
    InteractionContext ictx{ctx, viewport_, *root_, outputSize};
    return chain_.dispatch(event, ictx);
  }

}  // namespace fluir::editor
