#pragma once

#include "editor/tools/tool.hpp"

namespace fluir::editor {

  /** Left-drag from a terminal to a compatible terminal adds a conduit; release elsewhere or Escape cancels. */
  class ConduitTool : public Tool {
   public:
    bool onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) override;
    bool capturing() const override { return active_; }
    void cancel(EditorState&) override { active_ = false; }
    void draw(const Subview& view, const EditorState& state, std::span<const Box> boxes) const override;

   private:
    bool active_ = false;
    TerminalHit from_;
    Vec2 cursor_; /**< world */
  };

}  // namespace fluir::editor
