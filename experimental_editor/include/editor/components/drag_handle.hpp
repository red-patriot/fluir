#ifndef FLUIR_EDITOR_COMPONENTS_DRAG_HANDLE_HPP
#define FLUIR_EDITOR_COMPONENTS_DRAG_HANDLE_HPP

#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {
  Rect dragRect(const fluir::FlowGraphLocation& nodeLoc);

  class DragHandle {
   public:
    static constexpr double WIDTH = 3;
    static constexpr double HEIGHT = 3;

    DragHandle(Rect handleRect, FlowGraphLocation& position, Rect& bounds);

    bool onDragStart(const EditorContext& ctx, Vec2 worldPos);
    void onDrag(const EditorContext& ctx, Vec2 worldPos, Vec2 worldDelta);

    void draw(const Subview& view, const EditorContext& ctx, const Rect& nodeRect) const;

    const Rect& box(const EditorContext& ctx) const;

   private:
    Rect rect_;
    FlowGraphLocation& location_;
    Rect& bounds_;

    Vec2 accumulator_{}; /**< Accumulated diff while dragging */
  };
}  // namespace fluir::editor

#endif
