#include "editor/diagram/node.hpp"

namespace fluir::editor::diagram {

  FluirNode::FluirNode(const EditorContext& ctx, QGraphicsItem* parent) : QGraphicsItem(parent), ctx_(ctx) { }

  QRectF FluirNode::boundingRect() const {
    const Rect local = frame();
    return {local.x, local.y, local.w, local.h};
  }

  void FluirNode::place() {
    const double unit = ctx_.layout.unitPx;
    setPos(location().x * unit, location().y * unit);
    setZValue(location().z);
  }

  Rect FluirNode::frame() const {
    const double unit = ctx_.layout.unitPx;
    return {0, 0, location().width * unit, location().height * unit};
  }

}  // namespace fluir::editor::diagram
