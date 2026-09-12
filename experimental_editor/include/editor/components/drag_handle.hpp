#ifndef FLUIR_EDITOR_COMPONENTS_DRAG_HANDLE_HPP
#define FLUIR_EDITOR_COMPONENTS_DRAG_HANDLE_HPP

#include "compiler/models/location.hpp"
#include "editor/core/geometry.hpp"
#include "editor/gesture/grip.hpp"

namespace fluir::editor {
  // Deprecated: kept so existing tests keep compiling. Use editor/gesture/grip.hpp.
  inline Rect dragRect(const fluir::FlowGraphLocation& nodeLoc) { return dragGripRect(nodeLoc); }
}  // namespace fluir::editor

#endif
