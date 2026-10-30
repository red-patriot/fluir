#include "editor/view/graph_geometry.hpp"

namespace fluir::editor {

  Rect localRect(const FlowGraphLocation& loc, double unitPx) {
    return {static_cast<double>(loc.x) * unitPx,
            static_cast<double>(loc.y) * unitPx,
            static_cast<double>(loc.width) * unitPx,
            static_cast<double>(loc.height) * unitPx};
  }

  Vec2 bodyOrigin(Vec2 functionOrigin, double headerH) { return functionOrigin + Vec2{0.0, headerH}; }

  Rect contentRect(const Rect& frame, double headerH) {
    const Vec2 origin = bodyOrigin(frame.topLeft(), headerH);
    return {frame.x, origin.y, frame.w, frame.y + frame.h - origin.y};
  }

  Rect atOrigin(Vec2 origin, Rect local) { return {origin.x + local.x, origin.y + local.y, local.w, local.h}; }

  Rect dotRect(Vec2 anchor, double terminalDot) {
    return {anchor.x - terminalDot * 0.5, anchor.y - terminalDot * 0.5, terminalDot, terminalDot};
  }

}  // namespace fluir::editor
