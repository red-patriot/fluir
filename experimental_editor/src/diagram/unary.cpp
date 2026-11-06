#include "editor/diagram/unary.hpp"

#include <algorithm>
#include <QPainter>
#include <string>
#include <utility>
#include <vector>

#include "editor/core/choices.hpp"
#include "editor/diagram/terminal.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/view/draw/unary.hpp"

namespace fluir::editor::diagram {

  Unary::Unary(pt::Unary data, const EditorContext& ctx, QGraphicsItem* parent) :
    FluirNode(ctx, parent),
    data_(std::move(data)),
    terminals_(addTerminals(*this, draw::anchors(data_, frame(), ctx.layout), ctx)) {
    addFields(draw::labels(data_, frame(), ctx.layout), *this);
    init(draw::resizePart(data_));
  }

  Limits<Vec2i> Unary::sizeLimits() const { return draw::sizeLimits(data_); }

  void Unary::arrange() {
    moveTerminals(terminals_, draw::anchors(data_, frame(), ctx_.layout));
    placeFields(draw::labels(data_, frame(), ctx_.layout));
  }

  void Unary::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::shell(*painter, boundingRect(), draw::color(data_, ctx_.theme), ctx_);
  }

  std::string Unary::text(Field field) const {
    return field.kind == Field::Kind::Operator ? std::string{stringify(data_.op)} : std::string{};
  }

  bool Unary::accepts(Field field, std::string_view text) const {
    return field.kind == Field::Kind::Operator && std::ranges::contains(unaryOperators(), text);
  }

  std::vector<std::string> Unary::choices(Field field) const {
    return field.kind == Field::Kind::Operator ? unaryOperators() : std::vector<std::string>{};
  }

  void Unary::write(Field, std::string_view text) { data_.op = *operatorNamed(text); }

}  // namespace fluir::editor::diagram
