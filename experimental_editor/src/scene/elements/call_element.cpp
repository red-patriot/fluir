#include "editor/scene/elements/call_element.hpp"

#include <QPainter>
#include <utility>

#include "editor/scene/item_style.hpp"
#include "editor/view/draw/call.hpp"

namespace fluir::editor {

  CallElement::CallElement(FullID path, const EditorContext& ctx, QGraphicsItem* parent) :
    Element(std::move(path), ctx, parent) { }

  void CallElement::sync(const et::ParseTree& tree, const et::Call& node, const Rect& world) {
    place(world);
    color_ = draw::color(node, ctx_.theme);
    clearComponents();
    addLabels(tree, modelPath(), draw::labels(node, world, ctx_.layout));
    addGrips(world, draw::resizePart(node), moveGrip(world, ctx_.layout.unitPx));
    addTerminals(modelPath(), draw::anchors(node, world, ctx_.layout));
    update();
  }

  void CallElement::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::shell(*painter, boundingRect(), color_, ctx_);
  }

}  // namespace fluir::editor
