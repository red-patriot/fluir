#include "editor/diagram/node.hpp"

#include <functional>
#include <memory>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <utility>

#include "editor/commands/delete_selection.hpp"
#include "editor/core/field.hpp"
#include "editor/diagram/choice.hpp"
#include "editor/diagram/context_menu.hpp"
#include "editor/diagram/field_text.hpp"
#include "editor/diagram/move_grip.hpp"
#include "editor/diagram/resize_grip.hpp"
#include "editor/diagram/scope.hpp"
#include "editor/diagram/toggle.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor::diagram {

  FluirNode::FluirNode(const EditorContext& ctx, QGraphicsItem* parent) : QGraphicsItem(parent), ctx_(ctx) {
    setFlag(ItemIsSelectable);
  }

  void FluirNode::setLocation(const FlowGraphLocation& location) {
    prepareGeometryChange();
    storeLocation(location);
    place();
    arrange();
  }

  void FluirNode::selectAlone() {
    scene()->clearSelection();
    setSelected(true);
  }

  bool FluirNode::addMenuEntries(QMenu&, QPointF) { return true; }

  void FluirNode::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    auto menu = std::make_unique<QMenu>();
    if (!addMenuEntries(*menu, event->pos())) {
      event->ignore();
      return;
    }
    if (!menu->isEmpty()) {
      menu->addSeparator();
    }
    selectAlone();
    addEntry(*menu, "Delete", [this] { ModuleScene::of(*this).undoStack().push(new DeleteSelection({this})); });
    popUp(std::move(menu), event->screenPos());
  }

  std::string FluirNode::text(Field) const { return {}; }

  bool FluirNode::accepts(Field, std::string_view) const { return false; }

  bool FluirNode::setText(Field field, std::string_view text) {
    if (!accepts(field, text)) {
      return false;
    }
    write(field, text);
    for (FieldPart* part : fieldParts()) {
      if (part->field() == field) {
        part->refresh();
      }
    }
    update();
    return true;
  }

  std::vector<std::string> FluirNode::choices(Field) const { return {}; }

  void FluirNode::write(Field, std::string_view) { }

  QRectF FluirNode::boundingRect() const {
    const Rect local = frame();
    return {local.x, local.y, local.w, local.h};
  }

  void FluirNode::init(std::optional<ResizeAxes> axes) {
    moveGrip_ = new MoveGrip(ctx_, *this);
    if (axes) {
      resizeGrip_ = new ResizeGrip(*axes, ctx_, *this);
    }
    place();
  }

  Rect FluirNode::frame() const {
    const double unit = ctx_.layout.unitPx;
    return {0, 0, location().width * unit, location().height * unit};
  }

  void FluirNode::addFields(const std::vector<FieldLabel>& labels,
                            QGraphicsItem& parent,
                            const std::function<FieldKey(Field)>& key) {
    for (const FieldLabel& label : labels) {
      FieldKey keyed = key ? key(label.field) : [field = label.field] { return field; };
      FieldPart* part = nullptr;
      if (label.field.kind == Field::Kind::Bool) {
        part = new Toggle(*this, std::move(keyed), ctx_, parent);
      } else if (isChoice(label.field.kind)) {
        part = new Choice(*this, std::move(keyed), ctx_, parent);
      } else {
        part = new FieldText(*this, std::move(keyed), ctx_, parent);
      }
      part->place(label.rect);
    }
  }

  void FluirNode::placeFields(const std::vector<FieldLabel>& labels) {
    for (const FieldLabel& label : labels) {
      for (FieldPart* part : fieldParts()) {
        if (part->field() == label.field) {
          part->place(label.rect);
        }
      }
    }
  }

  std::vector<FieldPart*> FluirNode::fieldParts() const {
    std::vector<FieldPart*> parts;
    QList<QGraphicsItem*> pending = childItems();
    while (!pending.isEmpty()) {
      QGraphicsItem* item = pending.takeLast();
      // Nested nodes, and the scopes that hold them, own their own fields.
      if (dynamic_cast<FluirNode*>(item) != nullptr || dynamic_cast<Scope*>(item) != nullptr) {
        continue;
      }
      if (auto* part = dynamic_cast<FieldPart*>(item)) {
        parts.push_back(part);
      }
      pending.append(item->childItems());
    }
    return parts;
  }

  void FluirNode::place() {
    const double unit = ctx_.layout.unitPx;
    setPos(location().x * unit, location().y * unit);
    setZValue(location().z);
    moveGrip_->place(frame());
    if (resizeGrip_ != nullptr) {
      resizeGrip_->place(frame());
    }
  }

}  // namespace fluir::editor::diagram
