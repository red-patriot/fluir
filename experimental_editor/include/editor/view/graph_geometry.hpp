#ifndef FLUIR_EDITOR_VIEW_GRAPH_GEOMETRY_HPP
#define FLUIR_EDITOR_VIEW_GRAPH_GEOMETRY_HPP

#include <vector>

#include "compiler/models/location.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/tree.hpp"

namespace fluir::editor {

  /** A node's local-space rect: `loc`'s (x,y,w,h) each scaled by `unitPx`. */
  Rect localRect(const FlowGraphLocation& loc, double unitPx);

  /** Body content origin: a frame's top-left shifted down by the header band's height, since
   *  body content is rendered below the header so nodes don't render over it. */
  Vec2 bodyOrigin(Vec2 functionOrigin, double headerH);

  /** Shift a local-space rect by `origin`, keeping its size. */
  Rect atOrigin(Vec2 origin, Rect local);

  /** A `terminalDot`-sized square centered on `anchor`. */
  Rect dotRect(Vec2 anchor, double terminalDot);

  /** `tree`'s declarations, ordered ascending by (location.z, id). */
  std::vector<const et::Declaration*> sortedDeclarations(const et::ParseTree& tree);

  /** `block`'s nodes, ordered ascending by (location.z, id). */
  std::vector<const et::Node*> sortedNodes(const et::Block& block);

  /** `block`'s conduits, ordered ascending by id. */
  std::vector<const et::Conduit*> sortedConduits(const et::Block& block);

}  // namespace fluir::editor

#endif
