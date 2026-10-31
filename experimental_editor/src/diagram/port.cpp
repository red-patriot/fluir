#include "editor/diagram/port.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>

#include "editor/commands/move_port.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/diagram/grid.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor::diagram {

  // An input port's outside is an input and its inside a source; an output port mirrors it.
  Port::Port(const draw::Ports& ports,
             draw::PortRef ref,
             const Rect& frame,
             const EditorContext& ctx,
             QGraphicsItem* conditional) :
    QGraphicsItem(conditional), ctx_(ctx), ref_(ref), innerId_(draw::portOf(ports, ref)->innerId) {
    const Rect placed = draw::portRect(ports, ref, frame, ctx.layout);
    setFlag(ItemIsFocusable);
    place(placed);
    rect_ = QRectF(0, 0, placed.w, placed.h);
    const Vec2 left{0, placed.h / 2};
    const Vec2 right{placed.w, placed.h / 2};
    const int index = static_cast<int>(ref.index);
    outer_ = new Terminal(ref.output, index, ref.output ? right : left, ctx, this);
    if (innerId_ != INVALID_ID) {
      inner_ = new Terminal(!ref.output, 0, ref.output ? left : right, ctx, this);
    }
  }

  void Port::setRef(draw::PortRef ref) {
    ref_ = ref;
    outer_->setIndex(static_cast<int>(ref.index));
  }

  void Port::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    // The condition is the bool port; every other port is a plain value port.
    style::shell(*painter, rect_, ref_ == draw::PortRef{} ? ctx_.theme.boolNode : ctx_.theme.port, ctx_);
  }

  void Port::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
      event->ignore();
      return;
    }
    conditional().selectAlone();
    gesture_ = Gesture{.start = y(), .pressedAt = event->scenePos()};
    setFocus();
  }

  void Port::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (gesture_) {
      const int steps = gridSteps(gesture_->pressedAt, event->scenePos(), ctx_.layout.unitPx).y;
      const Limits<int> limits = draw::portYLimits(conditional().location().height, ctx_.layout);
      conditional().setPortY(ref_, clamp(gesture_->start + steps, limits));
    }
  }

  void Port::mouseReleaseEvent(QGraphicsSceneMouseEvent*) {
    if (gesture_ && y() != gesture_->start) {
      ModuleScene::of(*this).undoStack().push(new MovePort(conditional(), ref_, gesture_->start, y()));
    }
    gesture_.reset();
  }

  void Port::keyPressEvent(QKeyEvent* event) {
    if (!gesture_ || event->key() != Qt::Key_Escape) {
      event->ignore();
      return;
    }
    conditional().setPortY(ref_, gesture_->start);
    gesture_.reset();
  }

  Conditional& Port::conditional() const { return static_cast<Conditional&>(*parentItem()); }

  int Port::y() const { return draw::portOf(conditional().ports(), ref_)->y; }

}  // namespace fluir::editor::diagram
