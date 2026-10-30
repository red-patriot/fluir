#include "editor/diagram/terminal.hpp"

#include <cstddef>
#include <QPainter>

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
    setPos(anchor.x, anchor.y);
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

}  // namespace fluir::editor::diagram
