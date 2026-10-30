#include "editor/diagram/unary.hpp"

#include <QPainter>
#include <utility>

#include "editor/diagram/terminal.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/view/draw/unary.hpp"

namespace fluir::editor::diagram {

  Unary::Unary(pt::Unary data, const EditorContext& ctx, QGraphicsItem* parent) :
    FluirNode(ctx, parent), data_(std::move(data)) {
    place();
    addTerminals(*this, draw::anchors(data_, frame(), ctx.layout), ctx);
  }

  void Unary::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::shell(*painter, boundingRect(), draw::color(data_, ctx_.theme), ctx_);
  }

}  // namespace fluir::editor::diagram
