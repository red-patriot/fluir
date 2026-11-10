#ifndef FLUIR_EDITOR_COMMANDS_DELETE_CONDUIT_HPP
#define FLUIR_EDITOR_COMMANDS_DELETE_CONDUIT_HPP

#include <QUndoCommand>

#include "editor/commands/removed_element.hpp"
#include "editor/diagram/conduit.hpp"

namespace fluir::editor {

  /** Removes a conduit.*/
  class DeleteConduit : public QUndoCommand {
   public:
    explicit DeleteConduit(diagram::Conduit& conduit);

    void redo() override;
    void undo() override;

   private:
    diagram::Conduit& conduit_;
    RemovedElement removed_;
  };

}  // namespace fluir::editor

#endif
