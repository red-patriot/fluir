#ifndef FLUIR_EDITOR_VIEW_DRAW_CONDITIONAL_HPP
#define FLUIR_EDITOR_VIEW_DRAW_CONDITIONAL_HPP

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor {

  namespace draw {
    PortSet anchors(const pt::Conditional& node, const Rect& world, const EditorContext::Layout& layout) {
      assert(false && "UNIMPLEMENTED");
    }

    Color color(const pt::Conditional& node, const EditorContext::Theme& theme) { assert(false && "UNIMPLEMENTED"); }

    void draw(const pt::Conditional& node, const Rect& world, const Subview& view, const EditorContext& ctx) {
      assert(false && "UNIMPLEMENTED");
    }
  }  // namespace draw

}  // namespace fluir::editor
#endif
