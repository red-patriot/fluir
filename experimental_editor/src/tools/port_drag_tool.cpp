#include "editor/tools/port_drag_tool.hpp"

#include <algorithm>
#include <memory>
#include <variant>

#include "editor/transaction/move_port.hpp"
#include "editor/view/draw/conditional.hpp"

namespace fluir::editor {
  namespace {

    const et::Conditional* conditionalAt(const et::ParseTree& tree, const FullID& path) {
      return std::get_if<et::Conditional>(nodeAt(tree, path));
    }

    const et::BlockPort* portAt(const et::ParseTree& tree, const FullID& path, PortRef port) {
      const et::Conditional* conditional = conditionalAt(tree, path);
      return !conditional ? nullptr : portOf(*conditional, port);
    }

  }  // namespace

  bool PortDragTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    switch (event.type) {
      case InputEvent::Type::MouseDown:
        {
          if (active_ || event.button != InputEvent::Button::Left) {
            return false;
          }
          const Vec2 world = state.view.screenToWorld(event.pos);
          const Box* hit = hitAt(boxes, world);
          if (!hit || hit->part != Part::Port || !hit->port) {
            return false;
          }
          const et::BlockPort* port = portAt(state.editor.tree(), hit->path, *hit->port);
          if (port == nullptr) {
            return false;
          }
          active_ = true;
          path_ = hit->path;
          port_ = *hit->port;
          startY_ = port->y;
          deltaY_ = 0;
          steps_.start(world);
          return true;
        }

      case InputEvent::Type::MouseMove:
        {
          if (!active_) {
            return false;
          }
          // A port only slides along its wall, so only y steps count.
          const int step = steps_.advance(state.view.screenToWorld(event.pos), state.ctx.layout.unitPx).y;
          if (step != 0) {
            deltaY_ += step;
            reaim(state);
          }
          return true;
        }

      case InputEvent::Type::MouseUp:
        if (!active_ || event.button != InputEvent::Button::Left) {
          return false;
        }
        // A port that vanished under the gesture leaves nothing worth keeping.
        if (edit_ && portAt(state.editor.tree(), path_, port_)) {
          state.editor.record(std::move(edit_));
        }
        reset();
        return true;

      case InputEvent::Type::KeyDown:
        if (!active_ || event.key != InputEvent::Key::Escape) {
          return false;
        }
        cancel(state);
        return true;

      default:
        return false;
    }
  }

  void PortDragTool::cancel(EditorState& state) {
    if (edit_) {
      edit_->unexecute(state.editor.tree());
    }
    reset();
  }

  void PortDragTool::reaim(EditorState& state) {
    et::ParseTree& tree = state.editor.tree();
    if (edit_) {
      edit_->unexecute(tree);
      edit_.reset();
    }
    const et::Conditional* conditional = conditionalAt(tree, path_);
    if (!conditional) {
      return;
    }
    const Limits<int> limits = draw::portYLimits(*conditional, state.ctx.layout);
    std::unique_ptr<Transaction> next =
      movePortTo(path_, port_, std::clamp(startY_ + deltaY_, limits.lower, limits.upper));
    // Back at the start is a no-op, so there is then no live edit.
    if (next->execute(tree)) {
      edit_ = std::move(next);
    }
  }

  void PortDragTool::reset() {
    active_ = false;
    path_.clear();
    edit_.reset();
  }

}  // namespace fluir::editor
