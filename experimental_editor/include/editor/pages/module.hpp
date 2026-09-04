#pragma once

#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"

namespace fluir::editor {
  class ModulePage {
   public:
    ModulePage(EditorContext& ctx, Renderer& renderer);

    int start();
    int update(const std::vector<InputEvent>& events);
    int write();

   private:
    EditorContext& ctx_;
    Renderer& renderer_;
    Viewport view_;
    std::optional<fluir::pt::ParseTree> tree_;
    bool panning_ = false;
    bool spaceHeld_ = false;
    Vec2 lastPan_;

    void reset();
  };
}  // namespace fluir::editor
