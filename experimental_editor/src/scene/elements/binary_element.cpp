#include "editor/scene/elements/binary_element.hpp"

#include <QPainter>
#include <utility>

#include "editor/scene/item_style.hpp"
#include "editor/view/draw/binary.hpp"

namespace fluir::editor {

  BinaryElement::BinaryElement(FullID path, const EditorContext& ctx, QGraphicsItem* parent) :
    Element(std::move(path), ctx, parent) { }

  void BinaryElement::sync(const et::ParseTree& tree, const et::Binary& node, const Rect& world) {
    place(world);
    color_ = draw::color(node, ctx_.theme);
    clearComponents();
    addLabels(tree, modelPath(), draw::labels(node, world, ctx_.layout));
    addGrips(world, draw::resizePart(node), moveGrip(world, ctx_.layout.unitPx));
    addTerminals(modelPath(), draw::anchors(node, world, ctx_.layout));
    update();
  }

  void BinaryElement::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::shell(*painter, boundingRect(), color_, ctx_);
  }

}  // namespace fluir::editor
