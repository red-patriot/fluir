#include "editor/scene/components/branch_arrow.hpp"

#include <QPainter>

#include "editor/scene/item_style.hpp"

namespace fluir::editor {

  BranchArrow::BranchArrow(const Rect& world, const EditorContext& ctx, QGraphicsItem* parent) :
    QGraphicsItem(parent), ctx_(ctx), rect_(style::place(*this, world)) { }

  void BranchArrow::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::icon(*painter, ":/images/branchArrow.svg", rect_, ctx_.theme.text);
  }

}  // namespace fluir::editor
