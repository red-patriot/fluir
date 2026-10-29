#include "editor/scene/elements/constant_element.hpp"

#include <QPainter>
#include <string>
#include <utility>
#include <variant>

#include "editor/core/literal_text.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/view/draw/constant.hpp"

namespace fluir::editor {

  ConstantElement::ConstantElement(FullID path, const EditorContext& ctx, QGraphicsItem* parent) :
    Element(std::move(path), ctx, parent) { }

  void ConstantElement::sync(const et::ParseTree& tree, const et::Constant& node, const Rect& world) {
    place(world);
    color_ = draw::color(node, ctx_.theme);
    tag_ = std::holds_alternative<fluir::literals_types::BOOL>(node.value) ? std::string{} :
                                                                             std::string{literalTypeName(node.value)};
    clearComponents();
    addLabels(tree, modelPath(), draw::labels(node, world, ctx_.layout));
    addGrips(world, draw::resizePart(node), moveGrip(world, ctx_.layout.unitPx));
    addTerminals(modelPath(), draw::anchors(node, world, ctx_.layout));
    update();
  }

  void ConstantElement::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::shell(*painter, boundingRect(), color_, ctx_);
    style::tag(*painter, boundingRect(), tag_, ctx_);
  }

}  // namespace fluir::editor
