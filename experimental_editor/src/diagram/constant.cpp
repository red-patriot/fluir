#include "editor/diagram/constant.hpp"

#include <QPainter>
#include <utility>
#include <variant>

#include "editor/core/literal_text.hpp"
#include "editor/diagram/terminal.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/view/draw/constant.hpp"

namespace fluir::editor::diagram {

  Constant::Constant(pt::Constant data, const EditorContext& ctx, QGraphicsItem* parent) :
    FluirNode(ctx, parent), data_(std::move(data)) {
    place();
    addTerminals(*this, draw::anchors(data_, frame(), ctx.layout), ctx);
  }

  void Constant::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::shell(*painter, boundingRect(), draw::color(data_, ctx_.theme), ctx_);
    if (!std::holds_alternative<fluir::literals_types::BOOL>(data_.value)) {
      style::tag(*painter, boundingRect(), literalTypeName(data_.value), ctx_);
    }
  }

}  // namespace fluir::editor::diagram
