#include "editor/diagram/binary.hpp"

#include <algorithm>
#include <QPainter>
#include <string>
#include <utility>
#include <vector>

#include "editor/core/choices.hpp"
#include "editor/diagram/terminal.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/view/draw/binary.hpp"

namespace fluir::editor::diagram {

  Binary::Binary(pt::Binary data, const EditorContext& ctx, QGraphicsItem* parent) :
    FluirNode(ctx, parent),
    data_(std::move(data)),
    terminals_(addTerminals(*this, draw::anchors(data_, frame(), ctx.layout), ctx)) {
    addFields(draw::labels(data_, frame(), ctx.layout), *this);
    init(draw::resizePart(data_));
  }

  Limits<Vec2i> Binary::sizeLimits() const { return draw::sizeLimits(data_); }

  void Binary::arrange() {
    moveTerminals(terminals_, draw::anchors(data_, frame(), ctx_.layout));
    placeFields(draw::labels(data_, frame(), ctx_.layout));
  }

  void Binary::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::shell(*painter, boundingRect(), draw::color(data_, ctx_.theme), ctx_);
  }

  std::string Binary::text(Field field) const {
    return field.kind == Field::Kind::Operator ? std::string{stringify(data_.op)} : std::string{};
  }

  bool Binary::accepts(Field field, std::string_view text) const {
    return field.kind == Field::Kind::Operator && std::ranges::contains(binaryOperators(), text);
  }

  std::vector<std::string> Binary::choices(Field field) const {
    return field.kind == Field::Kind::Operator ? binaryOperators() : std::vector<std::string>{};
  }

  void Binary::write(Field, std::string_view text) { data_.op = *operatorNamed(text); }

}  // namespace fluir::editor::diagram
