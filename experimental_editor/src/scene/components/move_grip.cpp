#include "editor/scene/components/move_grip.hpp"

#include <QPainter>

#include "editor/scene/item_style.hpp"

namespace fluir::editor {

  MoveGrip::MoveGrip(const Rect& world, const EditorContext& ctx, QGraphicsItem* parent) :
    QGraphicsItem(parent), ctx_(ctx), rect_(style::place(*this, world)) { }

  void MoveGrip::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::icon(*painter, ":/images/dragHandle.svg", rect_, ctx_.theme.border);
  }

}  // namespace fluir::editor
