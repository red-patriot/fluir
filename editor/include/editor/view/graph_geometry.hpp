#ifndef FLUIR_EDITOR_VIEW_GRAPH_GEOMETRY_HPP
#define FLUIR_EDITOR_VIEW_GRAPH_GEOMETRY_HPP

#include "compiler/models/location.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** A node's local-space rect: `loc`'s (x,y,w,h) each scaled by `unitPx`. */
  Rect localRect(const FlowGraphLocation& loc, double unitPx);

  /** Body content origin: a frame's top-left shifted down by the header band's height, since
   *  body content is rendered below the header so nodes don't render over it. */
  Vec2 bodyOrigin(Vec2 functionOrigin, double headerH);

  /** A container's region under its header band: where a function's body or a conditional's branch lays out. */
  Rect contentRect(const Rect& frame, double headerH);

  /** Shift a local-space rect by `origin`, keeping its size. */
  Rect atOrigin(Vec2 origin, Rect local);

  /** A `terminalDot`-sized square centered on `anchor`. */
  Rect dotRect(Vec2 anchor, double terminalDot);

}  // namespace fluir::editor

#endif
