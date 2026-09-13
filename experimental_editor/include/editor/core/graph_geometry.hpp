#pragma once

#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** A node's local-space rect: `loc`'s (x,y,w,h) each scaled by `unitPx`. */
  Rect localRect(const FlowGraphLocation& loc, double unitPx);

  /** Body content origin: a frame's top-left shifted down by the header band's height, since
   *  body content is rendered below the header so nodes don't render over it. */
  Vec2 bodyOrigin(Vec2 functionOrigin, double headerH);

  /** Shift a local-space rect by `origin`, keeping its size. */
  Rect atOrigin(Vec2 origin, Rect local);

  /** A `portDot`-sized square centered on `anchor`. */
  Rect dotRect(Vec2 anchor, double portDot);

  /** `tree`'s function declarations, ordered ascending by (location.z, id). */
  std::vector<const pt::FunctionDecl*> sortedFunctions(const pt::ParseTree& tree);

  /** `block`'s nodes, ordered ascending by (location.z, id). */
  std::vector<const pt::Node*> sortedNodes(const pt::Block& block);

  /** `block`'s conduits, ordered ascending by id. */
  std::vector<const pt::Conduit*> sortedConduits(const pt::Block& block);

}  // namespace fluir::editor
