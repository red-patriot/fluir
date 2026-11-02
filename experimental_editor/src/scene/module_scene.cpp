#include "editor/scene/module_scene.hpp"

#include <QKeyEvent>

#include "editor/commands/delete_conduit.hpp"
#include "editor/commands/delete_node.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/node.hpp"

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
      // One thing is selected at a time: a node or a conduit.
      QGraphicsItem* selected = selectedItems().front();
      if (auto* node = dynamic_cast<diagram::FluirNode*>(selected)) {
        undoStack_.push(new DeleteNode(*node));
      } else if (auto* conduit = dynamic_cast<diagram::Conduit*>(selected)) {
        undoStack_.push(new DeleteConduit(*conduit));
      }
      event->accept();
    }
  }

}  // namespace fluir::editor
