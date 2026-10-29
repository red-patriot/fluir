#include "editor/scene/elements/comment_element.hpp"

#include <QPainter>
#include <utility>

#include "editor/scene/item_style.hpp"
#include "editor/view/draw/comment.hpp"

namespace fluir::editor {

  CommentElement::CommentElement(FullID path, const EditorContext& ctx, QGraphicsItem* parent) :
    Element(std::move(path), ctx, parent) { }

  void CommentElement::sync(const et::ParseTree& tree, const et::Comment& node, const Rect& world) {
    place(world);
    color_ = draw::color(node, ctx_.theme);
    tag_ = std::string{"//"};
    clearComponents();
    addLabels(tree, modelPath(), draw::labels(node, world, ctx_.layout));
    addGrips(world, draw::resizePart(node), moveGrip(world, ctx_.layout.unitPx));
    addTerminals(modelPath(), draw::anchors(node, world, ctx_.layout));
    update();
  }

  void CommentElement::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    style::shell(*painter, boundingRect(), color_, ctx_);
    style::tag(*painter, QRectF(0, 0, boundingRect().width(), ctx_.layout.headerH()), tag_, ctx_);
  }

}  // namespace fluir::editor
