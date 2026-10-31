#include "editor/diagram/conduit.hpp"

#include <algorithm>
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QPen>
#include <utility>

#include "editor/scene/item_style.hpp"

namespace fluir::editor::diagram {
  namespace {

    constexpr double PEN_PX = 2;
    // How wide a band around its lines takes a press, in grid units.
    constexpr double HIT_UNITS = 1;
    // Conduits paint over their scope's nodes.
    constexpr double CONDUIT_Z = 1e6;

    QPointF centerOf(const Terminal& terminal) { return terminal.mapToScene(QPointF(0, 0)); }

  }  // namespace

  Conduit::Conduit(
    fluir::ID id, Terminal* source, std::vector<Terminal*> sinks, const EditorContext& ctx, QGraphicsItem* scope) :
    QGraphicsPathItem(scope),
    id_(id),
    source_(source),
    sinks_(std::move(sinks)),
    hitPx_(HIT_UNITS * ctx.layout.unitPx) {
    setPen(QPen(style::color(ctx.theme.conduit), PEN_PX));
    setFlag(ItemIsSelectable);
    setZValue(CONDUIT_Z);
    attach();
  }

  void Conduit::setSinks(std::vector<Terminal*> sinks) {
    detach();
    sinks_ = std::move(sinks);
    attach();
  }

  void Conduit::attach() {
    source_->attach(this);
    for (Terminal* sink : sinks_) {
      sink->attach(this);
    }
    route();
  }

  void Conduit::detach() {
    source_->detach(this);
    for (Terminal* sink : sinks_) {
      sink->detach(this);
    }
  }

  QPainterPath Conduit::shape() const {
    QPainterPathStroker stroker;
    stroker.setWidth(std::max(hitPx_, PEN_PX));
    return stroker.createStroke(path());
  }

  QRectF Conduit::boundingRect() const { return shape().boundingRect(); }

  bool Conduit::collidesWithPath(const QPainterPath& path, Qt::ItemSelectionMode mode) const {
    // A clipped item is hit-tested by its clip path, so its shape must be checked too.
    return QGraphicsPathItem::collidesWithPath(path, mode) && shape().intersects(path);
  }

  void Conduit::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    const auto under = [event](const Terminal* end) { return end->sceneBoundingRect().contains(event->scenePos()); };
    if (under(source_) || std::ranges::any_of(sinks_, under)) {
      event->ignore();
      return;
    }
    QGraphicsPathItem::mousePressEvent(event);
  }

  void Conduit::route() {
    QPainterPath path;
    for (const Terminal* sink : sinks_) {
      path.moveTo(mapFromScene(centerOf(*source_)));
      path.lineTo(mapFromScene(centerOf(*sink)));
    }
    setPath(path);
  }

}  // namespace fluir::editor::diagram
