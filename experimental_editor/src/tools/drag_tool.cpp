#include "editor/tools/drag_tool.hpp"

#include <algorithm>
#include <memory>
#include <optional>

#include "editor/core/node_access.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/move.hpp"
#include "editor/transaction/resize.hpp"
#include "editor/view/draw/comment.hpp"
#include "editor/view/draw/function.hpp"
#include "editor/view/node_view.hpp"

namespace fluir::editor {
  namespace {
    // What the function, top-level comment or node at `path` may be resized to; nullopt when it is gone.
    std::optional<Limits<Vec2i>> sizeLimitsAt(const et::ParseTree& tree, const FullID& path) {
      if (const et::FunctionDecl* fn = functionAt(tree, path)) {
        return draw::sizeLimits(*fn);
      }
      if (const et::Comment* comment = commentAt(tree, path)) {
        return draw::sizeLimits(*comment);
      }
      const et::Node* node = nodeAt(tree, path);
      return node == nullptr ? std::nullopt : std::optional{nodeSizeLimits(*node)};
    }

    bool isGrip(Part part) {
      return part == Part::MoveGrip || part == Part::ResizeX || part == Part::ResizeY || part == Part::ResizeXY;
    }

  }  // namespace

  bool DragTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    switch (event.type) {
      case InputEvent::Type::MouseDown:
        {
          if (active_ || event.button != InputEvent::Button::Left) {
            return false;
          }
          const Vec2 world = state.view.screenToWorld(event.pos);
          const Box* hit = hitAt(boxes, world);
          const FlowGraphLocation* loc = hit == nullptr ? nullptr : locationAt(state.editor.tree(), hit->path);
          if (loc == nullptr || !isGrip(hit->part)) {
            return false;
          }
          active_ = true;
          path_ = hit->path;
          part_ = hit->part;
          start_ = *loc;
          lastWorld_ = world;
          remainder_ = Vec2{};
          delta_ = Vec2i{};
          return true;
        }

      case InputEvent::Type::MouseMove:
        {
          if (!active_) {
            return false;
          }
          // Sub-unit motion carries over, so slow drags still step.
          const Vec2 world = state.view.screenToWorld(event.pos);
          const double unit = state.ctx.layout.unitPx;
          remainder_ = remainder_ + (world - lastWorld_);
          lastWorld_ = world;
          const Vec2i steps{static_cast<int>(remainder_.x / unit), static_cast<int>(remainder_.y / unit)};
          remainder_ = remainder_ - Vec2{steps.x * unit, steps.y * unit};
          if (steps != Vec2i{}) {
            delta_ = delta_ + steps;
            reaim(state);
          }
          return true;
        }

      case InputEvent::Type::MouseUp:
        if (!active_ || event.button != InputEvent::Button::Left) {
          return false;
        }
        // A path that vanished under the gesture leaves nothing worth keeping.
        if (edit_ != nullptr && locationAt(state.editor.tree(), path_) != nullptr) {
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

  void DragTool::cancel(EditorState& state) {
    if (edit_ != nullptr) {
      edit_->unexecute(state.editor.tree());
    }
    reset();
  }

  void DragTool::reaim(EditorState& state) {
    et::ParseTree& tree = state.editor.tree();
    if (edit_ != nullptr) {
      edit_->unexecute(tree);
      edit_.reset();
    }
    const std::optional<Limits<Vec2i>> limits = sizeLimitsAt(tree, path_);
    if (!limits) {
      return;
    }
    const Limits<Vec2i>& size = *limits;
    const int width = std::clamp(start_.width + delta_.x, size.lower.x, size.upper.x);
    const int height = std::clamp(start_.height + delta_.y, size.lower.y, size.upper.y);

    std::unique_ptr<Transaction> next;
    if (part_ == Part::MoveGrip) {
      next = moveTo(path_, start_.x + delta_.x, start_.y + delta_.y);
    } else if (part_ == Part::ResizeX) {
      next = resizeTo(path_, width, start_.height);
    } else if (part_ == Part::ResizeY) {
      next = resizeTo(path_, start_.width, height);
    } else {
      next = resizeTo(path_, width, height);
    }
    // Back at the start is a no-op, so there is then no live edit.
    if (next->execute(tree)) {
      edit_ = std::move(next);
    }
  }

  void DragTool::reset() {
    active_ = false;
    path_.clear();
    edit_.reset();
  }

}  // namespace fluir::editor
