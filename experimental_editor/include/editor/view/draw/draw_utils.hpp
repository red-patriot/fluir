#pragma once

#include <span>
#include <string_view>
#include <vector>

#include "editor/assets/images.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  /** A node's terminal anchors, in world space. */
  struct TerminalSet {
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

  /** `intrinsic`'s aspect scaled to fit inside `target`, centred. Degenerate input returns an empty rect at
   *  `target`'s centre. */
  Rect fitInto(Rect target, Vec2 intrinsic);

  /** Draws `tag` small along `box`'s bottom and, when non-empty, `text` full size after it. */
  void drawSplitLabel(
    const Subview& view, Rect box, std::string_view tag, std::string_view text, const EditorContext& ctx);

  namespace draw {

    /** `count` anchors down the vertical edge at `edgeX`; a lone one sits at the centre. */
    std::vector<Vec2> edgeAnchors(double edgeX, const Rect& rect, int count);

    /** Fills `world` with `fill`, then draws its border. */
    void drawShell(const Rect& world, Color fill, const Subview& view, const EditorContext& ctx);

    /** A border-colored dot at every input, then every output. */
    void drawTerminalDots(const TerminalSet& terminalSet, const Subview& view, const EditorContext& ctx);

    /** Draws `text` at `world`'s padded top-left; skipped when empty. */
    void drawTitle(std::string_view text, const Rect& world, const Subview& view, const EditorContext& ctx);

    /** `svg` fitted into `world` mapped through `view`, aspect preserved, recolored to `tint`. */
    void drawImage(SvgView svg, const Rect& world, const Subview& view, const Color& tint);

    /** A move grip's drag handle, fit into `rect`. */
    void drawMoveGrip(const Rect& rect, const Subview& view, const EditorContext& ctx);

    /** An XY resize handle, fit to `rect` */
    void drawXyResizeHandle(const Rect& rect, const Subview& view, const EditorContext& ctx);

    /** A draggable edge: `rect` filled solid, so the whole thick line reads as the grip. */
    void drawResizeEdge(const Rect& rect, const Subview& view, const EditorContext& ctx);
  }  // namespace draw

}  // namespace fluir::editor
