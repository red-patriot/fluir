#pragma once

#include "editor/tools/tool.hpp"

namespace fluir::editor {

  /** Middle-drag pans; the wheel zooms within the context's clamp. */
  class PanZoomTool : public Tool {
   public:
    bool onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) override;
    bool capturing() const override { return panning_; }
    void cancel(EditorState&) override { panning_ = false; }

   private:
    bool panning_ = false;
    Vec2 last_;
  };

}  // namespace fluir::editor
