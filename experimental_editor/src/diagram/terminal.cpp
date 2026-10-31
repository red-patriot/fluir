#include "editor/diagram/terminal.hpp"

#include <cstddef>
#include <QPainter>
#include <vector>

#include "editor/diagram/conduit.hpp"
#include "editor/scene/item_style.hpp"

namespace fluir::editor::diagram {
  namespace {

    // The hit square's side, in grid units.
    constexpr double HIT_UNITS = 1;

  }  // namespace

  Terminal::Terminal(bool output, int index, Vec2 anchor, const EditorContext& ctx, QGraphicsItem* owner) :
    QGraphicsItem(owner), ctx_(ctx), output_(output), index_(index) {
    const double side = HIT_UNITS * ctx.layout.unitPx;
    rect_ = QRectF(-side / 2, -side / 2, side, side);
    setFlag(ItemSendsScenePositionChanges);
    setAnchor(anchor);
  }

  void Terminal::attach(Conduit* conduit) { conduits_.push_back(conduit); }

  void Terminal::detach(Conduit* conduit) { std::erase(conduits_, conduit); }

  QVariant Terminal::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemScenePositionHasChanged) {
      for (Conduit* conduit : conduits_) {
        conduit->route();
      }
    }
    return QGraphicsItem::itemChange(change, value);
  }

  void Terminal::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    const double dot = ctx_.layout.terminalDot;
    painter->setPen(Qt::NoPen);
    painter->setBrush(style::color(ctx_.theme.text));
    painter->drawEllipse(QPointF(0, 0), dot / 2, dot / 2);
  }

  Terminals addTerminals(QGraphicsItem& owner, const TerminalSet& anchors, const EditorContext& ctx) {
    Terminals out;
    for (std::size_t i = 0; i < anchors.inputs.size(); ++i) {
      out.inputs.push_back(new Terminal(false, static_cast<int>(i), anchors.inputs[i], ctx, &owner));
    }
    for (std::size_t i = 0; i < anchors.outputs.size(); ++i) {
      out.outputs.push_back(new Terminal(true, static_cast<int>(i), anchors.outputs[i], ctx, &owner));
    }
    return out;
  }

  void moveTerminals(const Terminals& terminals, const TerminalSet& anchors) {
    for (std::size_t i = 0; i < terminals.inputs.size(); ++i) {
      terminals.inputs[i]->setAnchor(anchors.inputs[i]);
    }
    for (std::size_t i = 0; i < terminals.outputs.size(); ++i) {
      terminals.outputs[i]->setAnchor(anchors.outputs[i]);
    }
  }

}  // namespace fluir::editor::diagram
