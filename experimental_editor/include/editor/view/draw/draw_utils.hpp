#pragma once

#include <string_view>
#include <vector>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  /** A node's port anchors, in world space. */
  struct PortSet {
    std::vector<Vec2> inputs;
    std::vector<Vec2> outputs;
  };

  /** A label's small tag and main text regions in world space; glyph cells are world px, so the split does not depend
   *  on the view. */
  struct SplitLabel {
    Rect tag;
    Rect text;
  };

  SplitLabel splitLabel(Rect box, std::string_view tag, const EditorContext::Layout& layout);

  /** Draws `tag` small along `box`'s bottom and, when non-empty, `text` full size after it. */
  void drawSplitLabel(
    const Subview& view, Rect box, std::string_view tag, std::string_view text, const EditorContext& ctx);

  namespace draw {

    /** `count` anchors down the vertical edge at `edgeX`; a lone one sits at the centre. */
    std::vector<Vec2> edgeAnchors(double edgeX, const Rect& rect, int count);

    /** Fills `world` with `fill`, then draws its border. */
    void drawShell(const Rect& world, Color fill, const Subview& view, const EditorContext& ctx);

    /** A border-colored dot at every input, then every output. */
    void drawPortDots(const PortSet& portSet, const Subview& view, const EditorContext& ctx);

    /** Draws `text` at `world`'s padded top-left; skipped when empty. */
    void drawTitle(std::string_view text, const Rect& world, const Subview& view, const EditorContext& ctx);

  }  // namespace draw

}  // namespace fluir::editor
