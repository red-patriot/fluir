#pragma once

#include <optional>
#include <span>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** What a box is. Only Body and the grips are hittable. */
  enum class Part { Body, Frame, Rail, Wire, MoveGrip, ResizeX, ResizeXY };

  /** One laid-out piece of the graph, in world space. */
  struct Box {
    FullID path;
    Part part;
    /** A Wire runs from this rect's top-left to its (x + w, y + h) corner. */
    Rect world;
    std::optional<Rect> clip;
  };

  /** Every box `tree` draws as, in paint order. */
  std::vector<Box> layoutGraph(const pt::ParseTree& tree, const EditorContext::Layout& layout);

  /** The last-painted hittable box containing `world`, or nullptr. */
  const Box* hitAt(std::span<const Box> boxes, Vec2 world);

  /** Union of the top-level declarations' bodies; {0,0,0,0} when there are none. */
  Rect graphBounds(std::span<const Box> boxes);

}  // namespace fluir::editor
