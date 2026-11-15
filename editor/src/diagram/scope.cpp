#include "editor/diagram/scope.hpp"

#include <algorithm>
#include <cstddef>
#include <QGraphicsScene>
#include <QList>
#include <QPointF>
#include <QString>
#include <QStringList>
#include <utility>
#include <vector>

#include "editor/commands/add_node.hpp"
#include "editor/core/intelligence.hpp"
#include "editor/core/scope_path.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/diagram/export.hpp"
#include "editor/diagram/grid.hpp"
#include "editor/diagram/node.hpp"
#include "editor/scene/completion_popup.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor::diagram {
  namespace {

    // Whether a part of the container lies under `scope` at `scenePos`.
    bool overPart(const Scope& scope, QPointF scenePos) {
      const QList<QGraphicsItem*> under = scope.scene()->items(scenePos);
      for (qsizetype i = under.indexOf(&scope) + 1; i < under.size(); ++i) {
        if (!under[i]->isAncestorOf(&scope)) {
          return true;
        }
      }
      return false;
    }

  }  // namespace

  Scope::Scope(const Rect& local, QGraphicsItem* container) : QGraphicsItem(container) {
    setFlag(ItemClipsChildrenToShape);
    setFlag(ItemHasNoContents);
    setRect(local);
  }

  void Scope::setRect(const Rect& local) {
    prepareGeometryChange();
    rect_ = QRectF(0, 0, local.w, local.h);
    setPos(local.x, local.y);
  }

  void Scope::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) { }

  void Scope::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    if (overPart(*this, event->scenePos())) {
      event->ignore();
      return;
    }
    const ModuleScene& scene = ModuleScene::of(*this);
    std::vector<Completion<pt::Node>> options = scene.intelligence().completionsAt(exportTree(scene), pathOf(*this));
    if (options.empty()) {
      event->ignore();
      return;
    }
    QStringList labels;
    for (const Completion<pt::Node>& option : options) {
      labels << QString::fromStdString(option.label);
    }
    const Vec2i at = nearestGridPoint(event->pos(), scene.context().layout.unitPx);
    popUpCompletions(std::move(labels), event->screenPos(), [this, options = std::move(options), at](qsizetype picked) {
      ModuleScene::of(*this).undoStack().push(
        new AddNode(*this, options[static_cast<std::size_t>(picked)].prototype, at));
    });
  }

  fluir::FullID pathOf(const Scope& scope) {
    fluir::FullID path;
    const Scope* current = &scope;
    while (const auto* conditional = dynamic_cast<const Conditional*>(current->parentItem())) {
      path.push_back(current == conditional->thenBranch() ? THEN_BRANCH_ID : ELSE_BRANCH_ID);
      path.push_back(conditional->id());
      current = static_cast<const Scope*>(conditional->parentItem());
    }
    path.push_back(static_cast<const FluirNode&>(*current->parentItem()).id());
    std::ranges::reverse(path);
    return path;
  }

}  // namespace fluir::editor::diagram
