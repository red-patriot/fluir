#ifndef FLUIR_EDITOR_VIEW_DRAW_CONDITIONAL_HPP
#define FLUIR_EDITOR_VIEW_DRAW_CONDITIONAL_HPP

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor {

  namespace draw {
    // A conditional carries no wall ports yet; wiring through scope ports is Phase 2.
    inline PortSet anchors(const pt::Conditional&, const Rect&, const EditorContext::Layout&) { return {}; }

    Color color(const pt::Conditional& node, const EditorContext::Theme& theme) { assert(false && "UNIMPLEMENTED"); }

    void draw(const pt::Conditional& node, const Rect& world, const Subview& view, const EditorContext& ctx) {
      assert(false && "UNIMPLEMENTED");
    }
  }  // namespace draw

}  // namespace fluir::editor
#endif
