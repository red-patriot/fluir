#include "editor/scene/components/scope_region.hpp"

#include "editor/scene/item_style.hpp"

namespace fluir::editor {

  ScopeRegion::ScopeRegion(QGraphicsItem* parent) : QGraphicsItem(parent) {
    setFlag(ItemClipsChildrenToShape);
    setFlag(ItemHasNoContents);
  }

  void ScopeRegion::place(const Rect& world) {
    prepareGeometryChange();
    rect_ = style::place(*this, world);
  }

  void ScopeRegion::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) { }

}  // namespace fluir::editor
