#include "editor/scene/components/resize_grip.hpp"

#include <QPainter>

#include "editor/scene/item_style.hpp"

namespace fluir::editor {

  ResizeGrip::ResizeGrip(Part part, const Rect& world, const EditorContext& ctx, QGraphicsItem* parent) :
    QGraphicsItem(parent), ctx_(ctx), rect_(style::place(*this, world)), part_(part) { }

  void ResizeGrip::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    // A corner grip is an icon; an edge grip is the whole thick line.
    if (part_ == Part::ResizeXY) {
      style::icon(*painter, ":/images/XYResizeHandle.svg", rect_, ctx_.theme.border);
    } else {
      painter->fillRect(rect_, style::color(ctx_.theme.border));
    }
  }

}  // namespace fluir::editor
