#ifndef FLUIR_EDITOR_PAGES_MODULE_HPP
#define FLUIR_EDITOR_PAGES_MODULE_HPP

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {
  class ModulePage {
   public:
    ModulePage(EditorContext& ctx, Renderer& renderer);

    int run();

   private:
    EditorContext& ctx_;
    Renderer& renderer_;
    Viewport view_;
    std::optional<fluir::pt::ParseTree> tree_;

    void reset();
    void redraw();
  };
}  // namespace fluir::editor

#endif
