#include "editor/scene/elements/conduit_element.hpp"

#include <QPainterPath>
#include <QPen>
#include <utility>

#include "editor/scene/item_style.hpp"

namespace fluir::editor {
  namespace {

    constexpr double PEN_PX = 2;

  }  // namespace

  ConduitElement::ConduitElement(FullID path, const EditorContext& ctx, QGraphicsItem* parent) :
    QGraphicsPathItem(parent), path_(std::move(path)) {
    setPen(QPen(style::color(ctx.theme.conduit), PEN_PX));
  }

  void ConduitElement::sync(const std::vector<std::pair<Vec2, Vec2>>& wires) {
    QPainterPath path;
    for (const auto& [from, to] : wires) {
      path.moveTo(mapFromScene(QPointF(from.x, from.y)));
      path.lineTo(mapFromScene(QPointF(to.x, to.y)));
    }
    setPath(path);
  }

}  // namespace fluir::editor
