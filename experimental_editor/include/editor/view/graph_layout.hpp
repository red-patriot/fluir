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

  /** What a box is. Frame and Wire are not hittable. Header and Label paint nothing; a Label sits over its owner. */
  enum class Part { Body, Branch, Frame, Header, Rail, Label, Wire, MoveGrip, ResizeX, ResizeY, ResizeXY, Terminal };

  /** One laid-out piece of the graph, in world space. */
  struct Box {
    FullID path;
    Part part;
    /** A Wire runs from this rect's top-left to its (x + w, y + h) corner. */
    Rect world;
    std::optional<Rect> clip;
    /** A Label's field; its path is the one `core/fields` takes. */
    std::optional<Field> field;
  };

  /** Every box `tree` draws as, in paint order. */
  std::vector<Box> layoutGraph(const et::ParseTree& tree, const EditorContext::Layout& layout);

  /** The last-painted hittable box containing `world`, looking through Labels, or nullptr. */
  const Box* hitAt(std::span<const Box> boxes, Vec2 world);

  /** The Label at `world` when it is the last-painted hittable box there, or nullptr. */
  const Box* labelAt(std::span<const Box> boxes, Vec2 world);

  /** A terminal: its node or rail `path`, side, and index on that side. */
  struct TerminalHit {
    FullID path;
    bool output = false;
    int index = 0;
    Vec2 anchor;
  };

  /** The top-painted terminal whose hit square contains `world`, honouring clips. */
  std::optional<TerminalHit> terminalAt(const et::ParseTree& tree,
                                        std::span<const Box> boxes,
                                        Vec2 world,
                                        const EditorContext::Layout& layout);

  /** Union of the top-level declarations' bodies; {0,0,0,0} when there are none. */
  Rect graphBounds(std::span<const Box> boxes);

}  // namespace fluir::editor

#endif
