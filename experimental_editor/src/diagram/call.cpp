#include "editor/diagram/call.hpp"

#include <algorithm>
#include <QPainter>
#include <string>
#include <utility>

#include "editor/core/identifier.hpp"
#include "editor/diagram/terminal.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/view/draw/call.hpp"

namespace fluir::editor::diagram {

  Call::Call(pt::Call data, const EditorContext& ctx, QGraphicsItem* parent) :
    FluirNode(ctx, parent),
    data_(std::move(data)),
    terminals_(addTerminals(*this, draw::anchors(data_, frame(), ctx.layout), ctx)) {
    addFields(draw::labels(data_, frame(), ctx.layout), *this);
    init(draw::resizePart(data_));
  }

  Limits<Vec2i> Call::sizeLimits() const { return draw::sizeLimits(data_); }

  void Call::arrange() {
    moveTerminals(terminals_, draw::anchors(data_, frame(), ctx_.layout));
    placeFields(draw::labels(data_, frame(), ctx_.layout));
  }

  void Call::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::shell(*painter, boundingRect(), draw::color(data_, ctx_.theme), ctx_);
  }

  namespace {

    // The argument named by `field.index`, if any.
    template <typename Call>
    auto* argumentAt(Call& call, Field field) {
      const auto it = std::ranges::find(call.arguments, field.index, &pt::Call::Argument::index);
      return it == call.arguments.end() ? nullptr : &*it;
    }

  }  // namespace

  std::string Call::text(Field field) const {
    if (field.kind == Field::Kind::Target) {
      return data_.target;
    }
    const auto* argument = field.kind == Field::Kind::Arg ? argumentAt(data_, field) : nullptr;
    return argument == nullptr ? "" : argument->name;
  }

  bool Call::accepts(Field field, std::string_view text) const {
    const bool named =
      field.kind == Field::Kind::Target || (field.kind == Field::Kind::Arg && argumentAt(data_, field) != nullptr);
    return named && isValidIdentifier(text);
  }

  void Call::write(Field field, std::string_view text) {
    if (field.kind == Field::Kind::Target) {
      data_.target = text;
    } else {
      argumentAt(data_, field)->name = text;
    }
  }

}  // namespace fluir::editor::diagram
