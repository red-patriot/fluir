#include "editor/scene/elements/conditional_element.hpp"

#include <QPainter>
#include <utility>

#include "editor/core/tree_path.hpp"
#include "editor/scene/components/branch_arrow.hpp"
#include "editor/scene/components/frame.hpp"
#include "editor/scene/components/port.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/view/draw/conditional.hpp"

namespace fluir::editor {

  ConditionalElement::ConditionalElement(FullID path, const EditorContext& ctx, QGraphicsItem* parent) :
    Element(std::move(path), ctx, parent) { }

  void ConditionalElement::sync(const et::ParseTree&, const et::Conditional& node, const Rect& world) {
    const EditorContext::Layout& layout = ctx_.layout;
    const Rect header{world.x, world.y, world.w, layout.headerH()};
    const std::string_view tag = draw::branchTag(node.annotation.shownBranch);
    place(world);
    clearComponents();
    addComponent<Frame>(
      world, draw::color(node, ctx_.theme), QString::fromUtf8(tag.data(), static_cast<qsizetype>(tag.size())), ctx_);
    addComponent<BranchArrow>(draw::branchArrowRect(header, layout), ctx_);
    for (const PortRef ref : portRefs(node)) {
      addComponent<Port>(ref, draw::portRect(node, ref, world, layout), ctx_);
    }
    addTerminals(modelPath(), draw::anchors(node, world, layout));
    addGrips(world, draw::resizePart(node), moveGrip(header, layout.unitPx));
  }

  void ConditionalElement::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->fillRect(boundingRect(), style::color(ctx_.theme.background));
  }

}  // namespace fluir::editor
