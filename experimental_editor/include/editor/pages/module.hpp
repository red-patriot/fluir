#pragma once

#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/actors/scene.hpp"
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

    // Test-only observability: lets tests verify a Left click actually
    // dispatched to the hit actor (Actor::onClick has no other externally
    // visible effect through this page's API).
    const GraphScene& scene() const { return scene_; }

   private:
    EditorContext& ctx_;
    Renderer& renderer_;
    Viewport view_;
    std::optional<fluir::pt::ParseTree> tree_;
    GraphScene scene_;
    bool panning_ = false;
    bool spaceHeld_ = false;
    Vec2 lastPan_;

    void reset();
  };
}  // namespace fluir::editor
