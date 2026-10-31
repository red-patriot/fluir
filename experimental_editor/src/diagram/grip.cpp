#include "editor/diagram/grip.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

#include "editor/commands/place_node.hpp"
#include "editor/diagram/grid.hpp"
#include "editor/diagram/node.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor::diagram {
  namespace {

    // Grips paint and take presses over their node's other parts.
    constexpr double GRIP_Z = 2;

  }  // namespace

  Grip::Grip(const EditorContext& ctx, FluirNode& node) : QGraphicsItem(&node), ctx_(ctx), node_(node) {
    setFlag(ItemIsFocusable);
    setZValue(GRIP_Z);
  }

  void Grip::setRect(const Rect& rect) {
    prepareGeometryChange();
    rect_ = style::rect(rect);
  }

  void Grip::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
      event->ignore();
      return;
    }
    node_.selectAlone();
    gesture_ = Gesture{.start = node_.location(), .pressedAt = event->scenePos()};
    setFocus();
  }

  void Grip::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (gesture_) {
      node_.setLocation(
        dragged(gesture_->start, gridSteps(gesture_->pressedAt, event->scenePos(), ctx_.layout.unitPx)));
    }
  }

  void Grip::mouseReleaseEvent(QGraphicsSceneMouseEvent*) {
    if (gesture_ && node_.location() != gesture_->start) {
      ModuleScene::of(*this).undoStack().push(new PlaceNode(node_, gesture_->start, node_.location()));
    }
    gesture_.reset();
  }

  void Grip::keyPressEvent(QKeyEvent* event) {
    if (!gesture_ || event->key() != Qt::Key_Escape) {
      event->ignore();
      return;
    }
    node_.setLocation(gesture_->start);
    gesture_.reset();
  }

}  // namespace fluir::editor::diagram
