#include "editor/diagram/conditional.hpp"

#include <QPainter>
#include <utility>

#include "editor/diagram/branch_arrow.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/view/graph_geometry.hpp"

namespace fluir::editor::diagram {
  namespace {

    // Ports paint over the shown branch.
    constexpr double PORT_Z = 1;

  }  // namespace

  Conditional::Conditional(
    fluir::ID id, FlowGraphLocation location, draw::Ports ports, const EditorContext& ctx, QGraphicsItem* parent) :
    FluirNode(ctx, parent), id_(id), location_(location), ports_(std::move(ports)) {
    const Rect content = contentRect(frame(), ctx.layout.headerH());
    then_ = new Scope(content, this);
    else_ = new Scope(content, this);
    else_->setVisible(false);
    for (const draw::PortRef ref : draw::portRefs(ports_)) {
      wallPorts_.push_back(new Port(ports_, ref, frame(), ctx, this));
      wallPorts_.back()->setZValue(PORT_Z);
    }
    new BranchArrow(ctx, *this);
    init(draw::resizePart(ports_));
  }

  void Conditional::switchBranch() {
    then_->setVisible(!then_->isVisible());
    else_->setVisible(!else_->isVisible());
    update();
  }

  void Conditional::setPortY(draw::PortRef port, int y) {
    draw::portOf(ports_, port)->y = y;
    for (Port* wallPort : wallPorts_) {
      if (wallPort->ref() == port) {
        wallPort->place(portRect(port));
      }
    }
  }

  void Conditional::arrange() {
    const Rect content = contentRect(frame(), ctx_.layout.headerH());
    then_->setRect(content);
    else_->setRect(content);
    for (Port* port : wallPorts_) {
      port->place(portRect(port->ref()));
    }
  }

  Rect Conditional::portRect(draw::PortRef port) const { return draw::portRect(ports_, port, frame(), ctx_.layout); }

  void Conditional::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    const QRectF band(0, 0, boundingRect().width(), ctx_.layout.headerH());
    painter->fillRect(boundingRect(), style::color(ctx_.theme.background));
    painter->fillRect(band, style::color(draw::color(ports_, ctx_.theme)));
    painter->setPen(QPen(style::color(ctx_.theme.border), 0));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect());
    style::tag(*painter, band, else_->isVisible() ? draw::ELSE_TAG : draw::THEN_TAG, ctx_);
  }

}  // namespace fluir::editor::diagram
