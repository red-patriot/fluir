#include "editor/scene/elements/rail_element.hpp"

#include <QPainter>
#include <utility>

#include "editor/scene/item_style.hpp"
#include "editor/view/draw/function.hpp"

namespace fluir::editor {

  RailElement::RailElement(FullID path, const EditorContext& ctx, QGraphicsItem* parent) :
    Element(std::move(path), ctx, parent) { }

  void RailElement::sync(const et::ParseTree& tree, const et::FunctionDecl& fn, fluir::ID railId, const Rect& world) {
    place(world);
    color_ = draw::color(fn, ctx_.theme);
    clearComponents();
    // Rail labels edit the function, so they take its path.
    addLabels(tree, FullID{fn.id}, draw::labels(fn, railId, world, ctx_.layout));
    addTerminals(modelPath(), draw::anchors(fn, railId, world));
    update();
  }

  void RailElement::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::shell(*painter, boundingRect(), color_, ctx_);
  }

}  // namespace fluir::editor
