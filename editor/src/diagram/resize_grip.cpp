#include "editor/diagram/resize_grip.hpp"

#include <QPainter>

#include "editor/diagram/grid.hpp"
#include "editor/diagram/node.hpp"
#include "editor/scene/item_style.hpp"

namespace fluir::editor::diagram {

  void ResizeGrip::place(const Rect& frame) {
    const double unit = ctx_.layout.unitPx;
    setRect(axes_ == ResizeAxes::XY ? resizeCorner(frame, unit) : resizeEdge(frame, unit));
  }

  void ResizeGrip::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    // A corner grip is an icon; an edge grip is the whole thick line.
    if (axes_ == ResizeAxes::XY) {
      style::icon(*painter, ":/images/XYResizeHandle.svg", boundingRect(), ctx_.theme.border);
    } else {
      painter->fillRect(boundingRect(), style::color(ctx_.theme.border));
    }
  }

  FlowGraphLocation ResizeGrip::dragged(const FlowGraphLocation& start, Vec2i steps) const {
    const Limits<Vec2i> limits = node_.sizeLimits();
    FlowGraphLocation location = start;
    location.width = clamp(start.width + steps.x, {limits.lower.x, limits.upper.x});
    if (axes_ == ResizeAxes::XY) {
      location.height = clamp(start.height + steps.y, {limits.lower.y, limits.upper.y});
    }
    return location;
  }

}  // namespace fluir::editor::diagram
