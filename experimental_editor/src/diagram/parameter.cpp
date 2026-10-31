#include "editor/diagram/parameter.hpp"

#include <QPainter>

#include "editor/diagram/terminal.hpp"
#include "editor/scene/item_style.hpp"

namespace fluir::editor::diagram {

  Parameter::Parameter(const draw::Signature& signature,
                       fluir::ID id,
                       const Rect& frame,
                       const EditorContext& ctx,
                       QGraphicsItem* function) :
    QGraphicsItem(function), ctx_(ctx), color_(draw::color(signature, ctx.theme)), id_(id) {
    const Rect placed = draw::railRect(signature, id, frame, ctx.layout);
    place(placed);
    rect_ = QRectF(0, 0, placed.w, placed.h);
    setAcceptedMouseButtons(Qt::NoButton);
    addTerminals(*this, draw::anchors(signature, id, Rect{0, 0, placed.w, placed.h}), ctx);
  }

  void Parameter::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::shell(*painter, rect_, color_, ctx_);
  }

}  // namespace fluir::editor::diagram
