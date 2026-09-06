#ifndef FLUIR_EDITOR_PAGES_MODULE_HPP
#define FLUIR_EDITOR_PAGES_MODULE_HPP

#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/actors/scene.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/layer.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"
#include "editor/pages/page.hpp"
#include "header_bar.hpp"

namespace fluir::editor {
  class ModulePage : public Page {
   public:
    ModulePage(EditorContext& ctx, Renderer& renderer);

    int start() override;
    int update(const std::vector<InputEvent>& events) override;
    int draw() override;

    std::unique_ptr<Page> next() override;

    // Test-only observability: lets tests verify a Left click actually
    // dispatched to the hit actor (Actor::onClick has no other externally
    // visible effect through this page's API).
    const GraphScene& scene() const { return scene_; }

    // Test-only observability: lets tests locate/click the header's Exit
    // button without a second parallel exit path through ModulePage's API.
    const HeaderBar& header() const { return header_; }

   private:
    EditorContext& ctx_;
    Renderer& renderer_;
    Viewport view_;
    std::optional<fluir::pt::ParseTree> tree_;
    GraphScene scene_;
    HeaderBar header_;
    Layer hudLayer_;
    bool shouldClose = false;
    bool panning_ = false;
    bool spaceHeld_ = false;
    Vec2 lastPan_;

    void reset();
  };
}  // namespace fluir::editor

#endif
