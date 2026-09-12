#ifndef FLUIR_EDITOR_COMPONENTS_RESIZE_HANDLE_HPP
#define FLUIR_EDITOR_COMPONENTS_RESIZE_HANDLE_HPP

#include "compiler/models/location.hpp"
#include "editor/core/geometry.hpp"
#include "editor/gesture/grip.hpp"

namespace fluir::editor {
  // Deprecated: kept so existing tests keep compiling. Use editor/gesture/grip.hpp.
  struct HorizResizeHandle {
    explicit HorizResizeHandle(Limits<int>) { }
    Rect rect(const fluir::FlowGraphLocation& nodeLoc) const { return horizResizeGripRect(nodeLoc); }
  };

  // Deprecated: kept so existing tests keep compiling. Use editor/gesture/grip.hpp.
  struct XYResizeHandle {
    explicit XYResizeHandle(Limits<Vec2i>) { }
    Rect rect(const fluir::FlowGraphLocation& nodeLoc) const { return xyResizeGripRect(nodeLoc); }
  };
}  // namespace fluir::editor

#endif
