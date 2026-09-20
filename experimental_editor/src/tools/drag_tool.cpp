#include "editor/tools/drag_tool.hpp"

#include <algorithm>
#include <memory>
#include <variant>

#include "editor/core/tree_path.hpp"
#include "editor/transaction/move.hpp"
#include "editor/transaction/resize.hpp"

namespace fluir::editor {
  namespace {
    // Size limits in grid units. A node's height follows its content, so it is unbounded below.
    constexpr Limits<Vec2i> NODE_SIZE{.lower = Vec2i{4, 0}, .upper = Vec2i{1000, 1000}};
    constexpr Limits<Vec2i> FUNCTION_SIZE{.lower = Vec2i{15, 15}, .upper = Vec2i{1000, 1000}};
    // Tall enough that the corner grip never overlaps the move grip.
    constexpr Limits<Vec2i> COMMENT_SIZE{.lower = Vec2i{8, 8}, .upper = Vec2i{1000, 1000}};
    // A conditional's height follows its branches, so only its width is dragged.
    constexpr Limits<Vec2i> CONDITIONAL_SIZE{.lower = Vec2i{10, 0}, .upper = Vec2i{1000, 1000}};
    // Deep enough that a branch keeps room for a node under its own header
    constexpr Limits<Vec2i> BRANCH_SIZE{.lower = Vec2i{10, 10}, .upper = Vec2i{1000, 1000}};

    bool isComment(const pt::ParseTree& tree, const FullID& path) {
      const pt::Declaration* decl = declarationAt(tree, path);
      const pt::Node* node = nodeAt(tree, path);
      return (decl != nullptr && std::holds_alternative<pt::Comment>(*decl)) ||
             (node != nullptr && std::holds_alternative<pt::Comment>(*node));
    }

    bool isConditional(const pt::ParseTree& tree, const FullID& path) {
      return std::get_if<pt::Conditional>(nodeAt(tree, path)) != nullptr;
    }

    bool isGrip(Part part) { return part == Part::MoveGrip || part == Part::ResizeX || part == Part::ResizeXY; }

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
    pt::ParseTree& tree = state.editor.tree();
    if (edit_ != nullptr) {
      edit_->unexecute(tree);
      edit_.reset();
    }
    const Limits<Vec2i>& size = functionAt(tree, path_) != nullptr ? FUNCTION_SIZE :
                                isScopePath(path_)                 ? BRANCH_SIZE :
                                isConditional(tree, path_)         ? CONDITIONAL_SIZE :
                                isComment(tree, path_)             ? COMMENT_SIZE :
                                                                     NODE_SIZE;
    // A branch is only ever as wide as its conditional, so its grip drags height alone.
    const int wanted = isScopePath(path_) ? start_.width : start_.width + delta_.x;
    const int width = std::clamp(wanted, size.lower.x, size.upper.x);
    const int height = std::clamp(start_.height + delta_.y, size.lower.y, size.upper.y);

    std::unique_ptr<Transaction> next;
    if (part_ == Part::MoveGrip) {
      next = std::make_unique<MoveTransaction>(path_, start_.x + delta_.x, start_.y + delta_.y);
    } else if (part_ == Part::ResizeX) {
      next = std::make_unique<ResizeTransaction>(path_, width, start_.height);
    } else {
      next = std::make_unique<ResizeTransaction>(path_, width, height);
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
