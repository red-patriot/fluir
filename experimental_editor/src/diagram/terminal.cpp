#include "editor/diagram/terminal.hpp"

#include <algorithm>
#include <cstddef>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <vector>

#include "editor/commands/add_conduit.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/port.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor::diagram {
  namespace {

    // The hit square's side, in grid units.
    constexpr double HIT_UNITS = 1;
    // Terminals sit over their owner's fields.
    constexpr double TERMINAL_Z = 1;

  }  // namespace

  Terminal::Terminal(bool output, int index, Vec2 anchor, const EditorContext& ctx, QGraphicsItem* owner) :
    QGraphicsItem(owner), ctx_(ctx), output_(output), index_(index) {
    const double side = HIT_UNITS * ctx.layout.unitPx;
    rect_ = QRectF(-side / 2, -side / 2, side, side);
    setFlags(ItemSendsScenePositionChanges | ItemIsFocusable);
    setZValue(TERMINAL_Z);
    setAnchor(anchor);
  }

  Scope* Terminal::scope() const {
    QGraphicsItem* owner = parentItem();
    if (const auto* port = dynamic_cast<const Port*>(owner)) {
      const auto& conditional = static_cast<const Conditional&>(*port->parentItem());
      if (this == port->outer()) {
        return static_cast<Scope*>(conditional.parentItem());
      }
      return conditional.thenBranch()->isVisible() ? conditional.thenBranch() : conditional.elseBranch();
    }
    if (dynamic_cast<const FluirNode*>(owner) != nullptr) {
      return static_cast<Scope*>(owner->parentItem());
    }
    return static_cast<const Function&>(*owner->parentItem()).body();
  }

  const QGraphicsItem* Terminal::endpoint() const {
    const auto* port = dynamic_cast<const Port*>(parentItem());
    return port != nullptr && this == port->outer() ? port->parentItem() : parentItem();
  }

  bool Terminal::feeds(const Terminal& sink) const {
    return std::ranges::any_of(conduits_, [&](const Conduit* conduit) {
      return conduit->source() == this && std::ranges::contains(conduit->sinks(), &sink);
    });
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

  void Terminal::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
      event->ignore();
      return;
    }
    preview_ = new QGraphicsLineItem(QLineF(QPointF(0, 0), event->pos()), this);
    preview_->setPen(QPen(style::color(ctx_.theme.conduit), 0));
    setFocus();
  }

  void Terminal::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (preview_) {
      preview_->setLine(QLineF(QPointF(0, 0), event->pos()));
    }
  }

  void Terminal::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (!preview_) {
      return;
    }
    endPreview();
    Terminal* partner = partnerAt(event->scenePos());
    if (partner == nullptr) {
      return;
    }
    // Either end may start the drag; the conduit always runs output to input.
    Terminal& source = output_ ? *this : *partner;
    Terminal& sink = output_ ? *partner : *this;
    if (!source.feeds(sink)) {
      ModuleScene::of(*this).undoStack().push(new AddConduit(*scope(), source, sink));
    }
  }

  void Terminal::keyPressEvent(QKeyEvent* event) {
    if (!preview_ || event->key() != Qt::Key_Escape) {
      event->ignore();
      return;
    }
    endPreview();
  }

  void Terminal::endPreview() {
    delete preview_;
    preview_ = nullptr;
  }

  Terminal* Terminal::partnerAt(QPointF scenePos) const {
    for (QGraphicsItem* item : scene()->items(scenePos)) {
      auto* terminal = dynamic_cast<Terminal*>(item);
      if (terminal != nullptr && terminal->isVisible() && terminal->output_ != output_ &&
          terminal->endpoint() != endpoint() && terminal->scope() == scope()) {
        return terminal;
      }
    }
    return nullptr;
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
