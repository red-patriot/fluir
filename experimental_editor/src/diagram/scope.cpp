#include "editor/diagram/scope.hpp"

namespace fluir::editor::diagram {

  Scope::Scope(const Rect& local, QGraphicsItem* container) : QGraphicsItem(container), rect_(0, 0, local.w, local.h) {
    setFlag(ItemClipsChildrenToShape);
    setFlag(ItemHasNoContents);
    setPos(local.x, local.y);
  }

  void Scope::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) { }

}  // namespace fluir::editor::diagram
