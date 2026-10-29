#include "editor/scene/components/port.hpp"

#include <QPainter>

#include "editor/scene/item_style.hpp"

namespace fluir::editor {

  Port::Port(PortRef port, const Rect& world, const EditorContext& ctx, QGraphicsItem* parent) :
    QGraphicsItem(parent), ctx_(ctx), rect_(style::place(*this, world)), port_(port) { }

  void Port::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    // The condition is the bool port; every other port is a plain value port.
    style::shell(*painter, rect_, port_ == PortRef{} ? ctx_.theme.boolNode : ctx_.theme.port, ctx_);
  }

}  // namespace fluir::editor
