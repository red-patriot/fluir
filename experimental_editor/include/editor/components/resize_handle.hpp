#ifndef FLUIR_EDITOR_COMPONENTS_RESIZE_HANDLE_HPP
#define FLUIR_EDITOR_COMPONENTS_RESIZE_HANDLE_HPP

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {
  /** The grip a node's width is dragged by. */
  class HorizResizeHandle {
   public:
    static constexpr double WIDTH = 1;

    /** True if `parentLocalPos` lands on the bar drawn inside `nodeRect`. */
    bool onDragStart(const EditorContext& ctx, Vec2 parentLocalPos, const FlowGraphLocation& loc, const Rect& nodeRect);

    /** Accumulates `worldDelta`'s x into the preview, snapped to whole grid units. */
    void onDrag(const EditorContext& ctx, Vec2 worldDelta);

    /** Whole grid units this gesture has widened by, not yet committed. */
    int dw() const { return dw_; }

    /** `loc` widened by the live preview -- what layout and hit-testing must use. */
    FlowGraphLocation preview(const FlowGraphLocation& loc) const;

    /** Dispatches this gesture's edit, if it resized anything, and drops the preview. */
    void commit(const EditorContext& ctx, const FlowGraphLocation& loc, const fluir::FullID& id);

    /** Drops the preview without an edit. */
    void cancel();

    void draw(const Subview& view, const EditorContext& ctx, const FlowGraphLocation& loc, const Rect& nodeRect) const;

    /** The bar's rect in grid units, down `nodeLoc`'s right edge. */
    Rect rect(const fluir::FlowGraphLocation& nodeLoc) const;

   private:
    double accumulator_ = 0; /**< sub-unit remainder of the live gesture */
    int dw_ = 0;             /**< whole grid units widened, not yet committed */
    bool active_ = false;
  };

  /** The grip a node's width and height can be dragged by. */
  class XYResizeHandle {
   public:
    static constexpr double SIZE = 3;

    /** True if `parentLocalPos` lands on the bar drawn inside `nodeRect`. */
    bool onDragStart(const EditorContext& ctx, Vec2 parentLocalPos, const FlowGraphLocation& loc, const Rect& nodeRect);

    /** Accumulates `worldDelta`'s x into the preview, snapped to whole grid units. */
    void onDrag(const EditorContext& ctx, Vec2 worldDelta);

    /** Whole grid units this gesture has widened by, not yet committed. */
    int dw() const { return dw_; }
    /** Whole grid units this gesture has heightened by, not yet committed. */
    int dh() const { return dh_; }

    /** `loc` widened by the live preview -- what layout and hit-testing must use. */
    FlowGraphLocation preview(const FlowGraphLocation& loc) const;

    /** Dispatches this gesture's edit, if it resized anything, and drops the preview. */
    void commit(const EditorContext& ctx, const FlowGraphLocation& loc, const fluir::FullID& id);

    /** Drops the preview without an edit. */
    void cancel();

    void draw(const Subview& view, const EditorContext& ctx, const FlowGraphLocation& loc, const Rect& nodeRect) const;

    /** The bar's rect in grid units relative to `nodeLoc`. */
    Rect rect(const fluir::FlowGraphLocation& nodeLoc) const;

   private:
    Vec2 accumulator_ = {0.0, 0.0}; /**< sub-unit remainder of the live gesture */
    int dw_ = 0;                    /**< whole grid units widened, not yet committed */
    int dh_ = 0;                    /**< whole grid units heightened, not yet committed */
    bool active_ = false;
  };
}  // namespace fluir::editor

#endif
