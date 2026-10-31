#include "editor/scene/module_scene.hpp"

#include <QKeyEvent>

#include "editor/commands/delete_selection.hpp"

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
      undoStack_.push(new DeleteSelection(selectedItems()));
      event->accept();
    }
  }

}  // namespace fluir::editor
