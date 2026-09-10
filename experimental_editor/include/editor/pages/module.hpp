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
  /** The page to display an open module and edit it. */
  class ModulePage : public Page {
   public:
    ModulePage(EditorContext& ctx, Renderer& renderer);

    std::unique_ptr<Page> next() override;

    // Test-only observability: lets tests verify a Left click actually
    // dispatched to the hit actor (Actor::onClick has no other externally
    // visible effect through this page's API).
    const GraphScene& scene() const { return scene_; }

    // Test-only observability: lets tests locate/click the header's Exit
    // button without a second parallel exit path through ModulePage's API.
    const HeaderBar& header() const { return header_; }

   protected:
    std::vector<Layer*> layers() override { return {&hud_, &graph_}; }
    int onStart() override;
    bool onAppEvent(const InputEvent& event) override;
    void onResize() override { layoutChrome(); }
    void afterUpdate() override { scene_.layout(ctx_); }

   private:
    std::optional<fluir::pt::ParseTree> tree_;
    GraphScene scene_;
    HeaderBar header_;
    Layer hud_;   /**< screen space; wins over the graph */
    Layer graph_; /**< world space; owns the pan/zoom viewport */
    bool shouldClose = false;

    void reset();
    void layoutChrome();

    void onSave();
    void onSaveAs();
    void syncTreeFromScene();
    bool saveToPath(const std::filesystem::path& path);
  };
}  // namespace fluir::editor

#endif
