#ifndef FLUIR_EDITOR_VIEW_GRAPH_LAYOUT_HPP
#define FLUIR_EDITOR_VIEW_GRAPH_LAYOUT_HPP

#include <optional>
#include <span>
#include <vector>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/field.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/tree.hpp"

namespace fluir::editor {

  /**
   * What part of the diagram a box is.
   */
  enum class Part {
    Body,
    Branch,
    Frame,
    Header,
    Rail,
    Label,
    Wire,
    MoveGrip,
    ResizeX,
    ResizeY,
    ResizeXY,
    Terminal,
    Port
  };

  /** Which terminal a Terminal box is: its side, and its index on that side. */
  struct BoxTerminal {
    bool output = false;
    size_t index = 0;
  };

  /** One laid-out piece of the graph, in world space. */
  struct Box {
    FullID path;
    Part part;
    /** A Wire runs from this rect's top-left to its (x + w, y + h) corner. */
    Rect world;
    std::optional<Rect> clip;
    /** A Label's field; its path is the one `core/fields` takes. */
    std::optional<Field> field;
    /** Set only on a Terminal box, whose path is its endpoint's and whose world is centered on its anchor. */
    std::optional<BoxTerminal> terminal;
  };

  /** Every box `tree` draws as, in paint order. */
  std::vector<Box> layoutGraph(const et::ParseTree& tree, const EditorContext::Layout& layout);

  /** The last-painted hittable box containing `world`, looking through Labels, or nullptr. */
  const Box* hitAt(std::span<const Box> boxes, Vec2 world);

  /** The Label at `world` when it is the last-painted hittable box there, or nullptr. */
  const Box* labelAt(std::span<const Box> boxes, Vec2 world);

  /** A terminal: its endpoint `path` (block path + endpoint id), side, and index on that side. */
  struct TerminalHit {
    FullID path;
    bool output = false;
    int index = 0;
    Vec2 anchor;
  };

  /** The last-painted Terminal box containing `world`. */
  std::optional<TerminalHit> terminalAt(std::span<const Box> boxes, Vec2 world);

  /** Union of the top-level declarations' bodies; {0,0,0,0} when there are none. */
  Rect graphBounds(std::span<const Box> boxes);

}  // namespace fluir::editor

#endif
