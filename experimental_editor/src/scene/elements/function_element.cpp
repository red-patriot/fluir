#include "editor/scene/elements/function_element.hpp"

#include <QPainter>
#include <utility>

#include "editor/scene/components/frame.hpp"
#include "editor/scene/components/scope_region.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/view/draw/function.hpp"
#include "editor/view/graph_geometry.hpp"

namespace fluir::editor {

  FunctionElement::FunctionElement(FullID path, const EditorContext& ctx, QGraphicsItem* parent) :
    Element(std::move(path), ctx, parent), scope_(new ScopeRegion(this)) { }

  void FunctionElement::sync(const et::ParseTree& tree, const et::FunctionDecl& fn) {
    const EditorContext::Layout& layout = ctx_.layout;
    const Rect frame = localRect(fn.location, layout.unitPx);
    const Rect header{frame.x, frame.y, frame.w, layout.headerH()};
    place(frame);
    scope_->place(contentRect(frame, layout.headerH()));
    clearComponents();
    addComponent<Frame>(frame, draw::color(fn, ctx_.theme), QString::fromUtf8(draw::FN_TAG), ctx_);
    addLabels(tree, modelPath(), draw::labels(fn, frame, layout));
    addGrips(frame, Part::ResizeXY, moveGrip(header, layout.unitPx));
  }

  void FunctionElement::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->fillRect(boundingRect(), style::color(ctx_.theme.background));
  }

}  // namespace fluir::editor
