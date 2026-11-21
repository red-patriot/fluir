#include "editor/diagram/constant.hpp"

#include <algorithm>
#include <cctype>
#include <QPainter>
#include <string>
#include <utility>
#include <variant>

#include "editor/core/literal_text.hpp"
#include "editor/diagram/terminal.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/view/draw/constant.hpp"

namespace fluir::editor::diagram {

  Constant::Constant(pt::Constant data, const EditorContext& ctx, QGraphicsItem* parent) :
    FluirNode(ctx, parent),
    data_(std::move(data)),
    terminals_(addTerminals(*this, draw::anchors(data_, frame(), ctx.layout), ctx)) {
    addFields(draw::labels(data_, frame(), ctx.layout), *this);
    init(draw::resizePart(data_));
  }

  Limits<Vec2i> Constant::sizeLimits() const { return draw::sizeLimits(data_); }

  void Constant::arrange() {
    moveTerminals(terminals_, draw::anchors(data_, frame(), ctx_.layout));
    placeFields(draw::labels(data_, frame(), ctx_.layout));
  }

  void Constant::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::shell(*painter, boundingRect(), draw::color(data_, ctx_.theme), ctx_);
    if (data_.typeTag != pt::Constant::BOOL) {
      std::string name{literalTypeName(data_)};
      style::tag(*painter, boundingRect(), name, ctx_);
    }
  }

  std::string Constant::text(Field field) const {
    if (data_.typeTag == pt::Constant::BOOL) {
      return field.kind == Field::Kind::Bool ? data_.value : "";
    }
    return field.kind == Field::Kind::Literal ? data_.value : "";
  }

  std::string Constant::displayText(Field field) const {
    if (data_.typeTag != pt::Constant::BOOL && field.kind == Field::Kind::Literal) {
      return renderLiteral(data_);
    }
    return text(field);
  }

  bool Constant::accepts(Field field, std::string_view text) const {
    if (data_.typeTag == pt::Constant::BOOL) {
      return field.kind == Field::Kind::Bool && (text == "true" || text == "false");
    }
    if (field.kind != Field::Kind::Literal) {
      return false;
    }
    return isValid(data_.typeTag, text);  // TODO: Handle string that isn't the right format
  }

  void Constant::write(Field field, std::string_view text) {
    // TODO: Handle string that isn't the right format for its type
    data_.value = text;
  }

}  // namespace fluir::editor::diagram
