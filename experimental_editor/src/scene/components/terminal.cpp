#include "editor/scene/components/terminal.hpp"

#include <QPainter>
#include <utility>

#include "editor/scene/item_style.hpp"
#include "editor/view/graph_geometry.hpp"

namespace fluir::editor {
  namespace {

    // The hit square's side, in grid units.
    constexpr double HIT_UNITS = 1;

  }  // namespace

  Terminal::Terminal(
    FullID path, bool output, std::size_t index, Vec2 anchor, const EditorContext& ctx, QGraphicsItem* parent) :
    QGraphicsItem(parent),
    ctx_(ctx),
    rect_(style::place(*this, dotRect(anchor, HIT_UNITS * ctx.layout.unitPx))),
    path_(std::move(path)),
    output_(output),
    index_(index) { }

  void Terminal::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    const double dot = ctx_.layout.terminalDot;
    painter->setPen(Qt::NoPen);
    painter->setBrush(style::color(ctx_.theme.text));
    painter->drawEllipse(rect_.center(), dot / 2, dot / 2);
  }

}  // namespace fluir::editor
