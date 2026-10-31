#include "editor/diagram/conduit.hpp"

#include <QPainterPath>
#include <QPen>
#include <utility>

#include "editor/scene/item_style.hpp"

namespace fluir::editor::diagram {
  namespace {

    constexpr double PEN_PX = 2;
    // Conduits paint over their scope's nodes.
    constexpr double CONDUIT_Z = 1e6;

    QPointF centerOf(const Terminal& terminal) { return terminal.mapToScene(QPointF(0, 0)); }

  }  // namespace

  Conduit::Conduit(
    fluir::ID id, Terminal* source, std::vector<Terminal*> sinks, const EditorContext& ctx, QGraphicsItem* scope) :
    QGraphicsPathItem(scope), id_(id), source_(source), sinks_(std::move(sinks)) {
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

  void Conduit::route() {
    QPainterPath path;
    for (const Terminal* sink : sinks_) {
      path.moveTo(mapFromScene(centerOf(*source_)));
      path.lineTo(mapFromScene(centerOf(*sink)));
    }
    setPath(path);
  }

}  // namespace fluir::editor::diagram
