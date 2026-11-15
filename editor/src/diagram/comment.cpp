#include "editor/diagram/comment.hpp"

#include <QPainter>
#include <string>
#include <utility>

#include "editor/diagram/terminal.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/view/draw/comment.hpp"

namespace fluir::editor::diagram {

  Comment::Comment(pt::Comment data, const EditorContext& ctx, QGraphicsItem* parent) :
    FluirNode(ctx, parent),
    data_(std::move(data)),
    terminals_(addTerminals(*this, draw::anchors(data_, frame(), ctx.layout), ctx)) {
    addFields(draw::labels(data_, frame(), ctx.layout), *this);
    init(draw::resizePart(data_));
  }

  Limits<Vec2i> Comment::sizeLimits() const { return draw::sizeLimits(data_); }

  void Comment::arrange() {
    moveTerminals(terminals_, draw::anchors(data_, frame(), ctx_.layout));
    placeFields(draw::labels(data_, frame(), ctx_.layout));
  }

  void Comment::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::shell(*painter, boundingRect(), draw::color(data_, ctx_.theme), ctx_);
    style::tag(*painter, QRectF(0, 0, boundingRect().width(), ctx_.layout.headerH()), "//", ctx_);
  }

  std::string Comment::text(Field field) const { return field.kind == Field::Kind::Text ? data_.text : ""; }

  bool Comment::accepts(Field field, std::string_view) const { return field.kind == Field::Kind::Text; }

  void Comment::write(Field, std::string_view text) { data_.text = text; }

}  // namespace fluir::editor::diagram
