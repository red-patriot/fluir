#include "editor/diagram/port.hpp"

#include <QPainter>

#include "editor/scene/item_style.hpp"

namespace fluir::editor::diagram {

  // An input port's outside is an input and its inside a source; an output port mirrors it.
  Port::Port(const draw::Ports& ports,
             draw::PortRef ref,
             const Rect& frame,
             const EditorContext& ctx,
             QGraphicsItem* conditional) :
    QGraphicsItem(conditional), ctx_(ctx), ref_(ref), innerId_(draw::portOf(ports, ref)->innerId) {
    const Rect placed = draw::portRect(ports, ref, frame, ctx.layout);
    setPos(placed.x, placed.y);
    rect_ = QRectF(0, 0, placed.w, placed.h);
    const Vec2 left{0, placed.h / 2};
    const Vec2 right{placed.w, placed.h / 2};
    const int index = static_cast<int>(ref.index);
    outer_ = new Terminal(ref.output, index, ref.output ? right : left, ctx, this);
    if (innerId_ != INVALID_ID) {
      inner_ = new Terminal(!ref.output, 0, ref.output ? left : right, ctx, this);
    }
  }

  void Port::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    // The condition is the bool port; every other port is a plain value port.
    style::shell(*painter, rect_, ref_ == draw::PortRef{} ? ctx_.theme.boolNode : ctx_.theme.port, ctx_);
  }

}  // namespace fluir::editor::diagram
