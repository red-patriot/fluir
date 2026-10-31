#ifndef FLUIR_EDITOR_COMMANDS_DELETE_SELECTION_HPP
#define FLUIR_EDITOR_COMMANDS_DELETE_SELECTION_HPP

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QList>
#include <QUndoCommand>
#include <vector>

#include "editor/diagram/conduit.hpp"
#include "editor/diagram/terminal.hpp"

namespace fluir::editor {

  /** Removes the selected nodes and conduits. A node takes the conduits it sources, leaves the conduits it sinks, and
   *  takes any conduit left with no sinks. While undone or redone, it keeps the removed items alive. */
  class DeleteSelection : public QUndoCommand {
   public:
    explicit DeleteSelection(const QList<QGraphicsItem*>& selected);
    ~DeleteSelection() override;

    void redo() override;
    void undo() override;

   private:
    struct Removal {
      QGraphicsItem* item;
      QGraphicsItem* parent;
    };

    struct Rewire {
      diagram::Conduit* conduit;
      std::vector<diagram::Terminal*> before;
      std::vector<diagram::Terminal*> after;
    };

    void remove(QGraphicsItem* item);

    QGraphicsScene* scene_;
    std::vector<Rewire> rewires_;
    std::vector<Removal> removals_;
    bool removed_ = false;
  };

}  // namespace fluir::editor

#endif
