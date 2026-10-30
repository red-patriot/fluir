#include "editor/diagram/comment.hpp"

#include <QPainter>
#include <utility>

#include "editor/diagram/terminal.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/view/draw/comment.hpp"

namespace fluir::editor::diagram {

  Comment::Comment(pt::Comment data, const EditorContext& ctx, QGraphicsItem* parent) :
    FluirNode(ctx, parent), data_(std::move(data)) {
    place();
    addTerminals(*this, draw::anchors(data_, frame(), ctx.layout), ctx);
  }

  void Comment::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::shell(*painter, boundingRect(), draw::color(data_, ctx_.theme), ctx_);
    style::tag(*painter, QRectF(0, 0, boundingRect().width(), ctx_.layout.headerH()), "//", ctx_);
  }

}  // namespace fluir::editor::diagram
