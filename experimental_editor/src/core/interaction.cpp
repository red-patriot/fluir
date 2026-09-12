#include "editor/core/interaction.hpp"

#include <cmath>
#include <optional>
#include <utility>

#include "editor/actors/scene.hpp"

namespace fluir::editor {
  namespace {

    struct Hit {
      Actor* actor = nullptr;
      Vec2 world;
      Vec2 local; /**< `world` in the hit actor's parent space */
    };

    Hit hitAt(InteractionContext& ctx, Vec2 screenPos) {
      Hit hit;
      hit.world = ctx.view.screenToWorld(screenPos);
      hit.actor = ctx.root.hitTest(hit.world);
      if (hit.actor != nullptr) {
        hit.local = hit.actor->toParentLocal(hit.world);
      }
      return hit;
    }

    bool isLeft(const InputEvent& event) { return event.button == InputEvent::Button::Left; }

    bool inTree(const Actor& root, const Actor* target) {
      if (&root == target) {
        return true;
      }
      for (const auto& child : root.children()) {
        if (inTree(*child, target)) {
          return true;
        }
      }
      return false;
    }

    // Rails and other chrome are hittable but not selectable, so walk up to the
    // nearest ancestor that names an id -- the enclosing frame.
    std::optional<fluir::FullID> selectionIdAt(Actor* hit) {
      for (Actor* actor = hit; actor != nullptr; actor = actor->parent()) {
        if (std::optional<fluir::FullID> id = actor->selectionId()) {
          return id;
        }
      }
      return std::nullopt;
    }

  }  // namespace

  void InteractionChain::add(std::unique_ptr<Interaction> interaction) { items_.push_back(std::move(interaction)); }

  bool InteractionChain::dispatch(const InputEvent& event, InteractionContext& ctx) {
    if (captured_ != nullptr) {
      const bool consumed = captured_->onEvent(event, ctx);
      if (!captured_->capturing()) {
        captured_ = nullptr;
      }
      return consumed;
    }
    for (const auto& item : items_) {
      if (!item->onEvent(event, ctx)) {
        continue;
      }
      if (event.type == InputEvent::Type::MouseDown && item->capturing()) {
        captured_ = item.get();
      }
      return true;
    }
    return false;
  }

  void InteractionChain::reset() {
    captured_ = nullptr;
    for (const auto& item : items_) {
      item->reset();
    }
  }

  void InteractionChain::dropFocus() {
    for (const auto& item : items_) {
      item->dropFocus();
    }
  }

  void PanZoomInteraction::reset() {
    panning_ = false;
    lastPan_ = Vec2{};
  }

  bool PanZoomInteraction::onEvent(const InputEvent& event, InteractionContext& ctx) {
    switch (event.type) {
      case InputEvent::Type::MouseDown:
        if (event.button == InputEvent::Button::Middle) {
          panning_ = true;
          lastPan_ = event.pos;
          return true;
        }
        return false;

      case InputEvent::Type::MouseMove:
        if (!panning_) {
          return false;
        }
        ctx.view.pan = ctx.view.pan + (event.pos - lastPan_);
        lastPan_ = event.pos;
        return true;

      case InputEvent::Type::MouseUp:
        if (!panning_) {
          return false;
        }
        if (event.button == InputEvent::Button::Middle) {
          panning_ = false;
          return true;
        }
        return false;

      case InputEvent::Type::Wheel:
        {
          const Viewport old = ctx.view;
          ctx.view.zoomAbout(event.pos, std::pow(ctx.editor.zoom.wheelStep, event.wheel.y));
          if (ctx.view.scale < ctx.editor.zoom.min || ctx.view.scale > ctx.editor.zoom.max) {
            ctx.view = old;
          }
          return true;
        }

      default:
        return false;
    }
  }

  void FocusInteraction::reset() {
    if (focused_ != nullptr) {
      focused_->onBlur();
      focused_ = nullptr;
    }
  }

  void FocusInteraction::dropStale(const Actor& root) {
    if (focused_ != nullptr && !inTree(root, focused_)) {
      reset();
    }
  }

  bool FocusInteraction::onEvent(const InputEvent& event, InteractionContext& ctx) {
    switch (event.type) {
      case InputEvent::Type::MouseDown:
        {
          if (!isLeft(event)) {
            return false;
          }
          dropStale(ctx.root);
          const Hit hit = hitAt(ctx, event.pos);
          // Re-offer to the actor that already has focus: it may re-aim, or
          // decline (a press on its grip) and give the focus up.
          if (hit.actor != nullptr && hit.actor == focused_) {
            if (!focused_->onFocus(ctx.editor, hit.local)) {
              reset();
            }
            return false;  // tracked, never consumed
          }
          reset();
          if (hit.actor != nullptr && hit.actor->onFocus(ctx.editor, hit.local)) {
            focused_ = hit.actor;
          }
          return false;  // tracked, never consumed
        }

      case InputEvent::Type::KeyDown:
        dropStale(ctx.root);
        return focused_ != nullptr && event.key && focused_->onKey(ctx.editor, *event.key);

      case InputEvent::Type::TextInput:
        dropStale(ctx.root);
        return focused_ != nullptr && focused_->onTextInput(ctx.editor, event.text);

      default:
        return false;
    }
  }

  void DragInteraction::reset() {
    if (dragActor_ != nullptr) {
      dragActor_->onDragCancel();
      dragActor_ = nullptr;
    }
  }

  bool DragInteraction::onEvent(const InputEvent& event, InteractionContext& ctx) {
    switch (event.type) {
      case InputEvent::Type::MouseDown:
        {
          if (!isLeft(event)) {
            return false;
          }
          const Hit hit = hitAt(ctx, event.pos);
          if (hit.actor == nullptr || !hit.actor->onDragStart(ctx.editor, hit.local)) {
            return false;
          }
          dragActor_ = hit.actor;
          lastDragWorld_ = hit.world;
          return true;
        }

      case InputEvent::Type::MouseMove:
        {
          if (dragActor_ == nullptr) {
            return false;
          }
          const Vec2 world = ctx.view.screenToWorld(event.pos);
          dragActor_->onDrag(ctx.editor, dragActor_->toParentLocal(world), world - lastDragWorld_);
          lastDragWorld_ = world;
          return true;
        }

      case InputEvent::Type::MouseUp:
        {
          if (dragActor_ == nullptr || !isLeft(event)) {
            return false;
          }
          const Vec2 world = ctx.view.screenToWorld(event.pos);
          dragActor_->onDragEnd(ctx.editor, dragActor_->toParentLocal(world));
          dragActor_ = nullptr;
          return true;
        }

      default:
        return false;
    }
  }

  bool SelectionInteraction::onEvent(const InputEvent& event, InteractionContext& ctx) {
    if (event.type != InputEvent::Type::MouseDown || !isLeft(event)) {
      return false;
    }
    if (const std::optional<fluir::FullID> id = selectionIdAt(hitAt(ctx, event.pos).actor)) {
      scene_.select(*id);
    } else {
      scene_.clearSelection();
    }
    return false;  // tracked, never consumed
  }

  bool ClickInteraction::onEvent(const InputEvent& event, InteractionContext& ctx) {
    if (event.type != InputEvent::Type::MouseDown || !isLeft(event)) {
      return false;
    }
    const Hit hit = hitAt(ctx, event.pos);
    if (hit.actor == nullptr) {
      return false;
    }
    hit.actor->onClick(hit.local);
    return true;
  }

}  // namespace fluir::editor
