#include "editor/scene/module_scene.hpp"

#include <cstddef>
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QString>
#include <QStringList>
#include <utility>
#include <vector>

#include "editor/commands/add_node.hpp"
#include "editor/commands/delete_conduit.hpp"
#include "editor/commands/delete_node.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/export.hpp"
#include "editor/diagram/grid.hpp"
#include "editor/diagram/node.hpp"
#include "editor/scene/completion_popup.hpp"

namespace fluir::editor {

  void ModuleScene::keyPressEvent(QKeyEvent* event) {
    QGraphicsScene::keyPressEvent(event);
    if (event->isAccepted()) {
      return;
    }
    if (event->key() == Qt::Key_Escape) {
      clearSelection();
      event->accept();
    } else if (event->key() == Qt::Key_Delete && !selectedItems().isEmpty()) {
      QGraphicsItem* selected = selectedItems().front();
      if (auto* node = dynamic_cast<diagram::FluirNode*>(selected)) {
        undoStack_.push(new DeleteNode(*node));
      } else if (auto* conduit = dynamic_cast<diagram::Conduit*>(selected)) {
        undoStack_.push(new DeleteConduit(*conduit));
      }
      event->accept();
    }
  }

  void ModuleScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    QGraphicsScene::contextMenuEvent(event);
    if (event->isAccepted() || !items(event->scenePos()).isEmpty()) {
      return;
    }
    std::vector<Completion<pt::Declaration>> options = intelligence_.topLevelCompletions(diagram::exportTree(*this));
    QStringList labels;
    for (const Completion<pt::Declaration>& option : options) {
      labels << QString::fromStdString(option.label);
    }
    const Vec2i at = diagram::nearestGridPoint(event->scenePos(), ctx_.layout.unitPx);
    popUpCompletions(std::move(labels), event->screenPos(), [this, options = std::move(options), at](qsizetype picked) {
      undoStack_.push(new AddNode(*this, options[static_cast<std::size_t>(picked)].prototype, at));
    });
    event->accept();
  }

}  // namespace fluir::editor
