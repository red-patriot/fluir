#include "editor/diagram/conditional.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <QGraphicsScene>
#include <QMenu>
#include <QPainter>
#include <utility>

#include "editor/commands/add_port.hpp"
#include "editor/diagram/branch_arrow.hpp"
#include "editor/diagram/context_menu.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/scene/module_scene.hpp"
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

  draw::PortRef Conditional::addPort(bool output, pt::BlockPort data) {
    side(output).push_back(data);
    const draw::PortRef ref{.output = output, .index = output ? ports_.outputs.size() - 1 : ports_.inputs.size()};
    wallPorts_.push_back(new Port(ports_, ref, frame(), ctx_, this));
    wallPorts_.back()->setZValue(PORT_Z);
    settlePorts();
    return ref;
  }

  Conditional::RemovedPort Conditional::removePort(draw::PortRef ref) {
    // Inputs start after the condition.
    const auto at = static_cast<std::ptrdiff_t>(ref.output ? ref.index : ref.index - 1);
    pt::BlockPorts& ports = side(ref.output);
    RemovedPort removed{.item = nullptr, .data = ports[static_cast<std::size_t>(at)], .ref = ref};
    ports.erase(std::next(ports.begin(), at));
    const auto item = std::ranges::find(wallPorts_, ref, &Port::ref);
    removed.item.reset(*item);
    wallPorts_.erase(item);
    removed.item->setParentItem(nullptr);
    scene()->removeItem(removed.item.get());
    shiftPorts(ref.output, ref.index + 1, -1);
    settlePorts();
    return removed;
  }

  void Conditional::restorePort(RemovedPort removed) {
    const auto at = static_cast<std::ptrdiff_t>(removed.ref.output ? removed.ref.index : removed.ref.index - 1);
    pt::BlockPorts& ports = side(removed.ref.output);
    shiftPorts(removed.ref.output, removed.ref.index, 1);
    ports.insert(std::next(ports.begin(), at), removed.data);
    removed.item->setRef(removed.ref);
    wallPorts_.push_back(removed.item.get());
    removed.item.release()->setParentItem(this);
    settlePorts();
  }

  void Conditional::shiftPorts(bool output, std::size_t from, int by) {
    for (Port* port : wallPorts_) {
      if (port->ref().output == output && port->ref().index >= from) {
        port->setRef({.output = output, .index = static_cast<std::size_t>(static_cast<int>(port->ref().index) + by)});
      }
    }
  }

  void Conditional::settlePorts() {
    std::ranges::sort(
      wallPorts_, {}, [](const Port* port) { return std::pair{port->ref().output, port->ref().index}; });
    for (Port* port : wallPorts_) {
      port->place(portRect(port->ref()));
    }
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

  bool Conditional::addMenuEntries(QMenu& menu, QPointF local) {
    if (local.y() >= frame().y + ctx_.layout.headerH()) {
      return false;
    }
    for (const bool output : {false, true}) {
      addEntry(menu, output ? "Add output" : "Add input", [this, output] {
        ModuleScene::of(*this).undoStack().push(new AddPort(*this, output));
      });
    }
    return true;
  }

}  // namespace fluir::editor::diagram
