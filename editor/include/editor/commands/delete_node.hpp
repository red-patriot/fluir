#ifndef FLUIR_EDITOR_COMMANDS_DELETE_NODE_HPP
#define FLUIR_EDITOR_COMMANDS_DELETE_NODE_HPP

#include <QUndoCommand>

#include "editor/commands/conduit_cut.hpp"
#include "editor/commands/removed_element.hpp"
#include "editor/diagram/node.hpp"

namespace fluir::editor {

  /** Removes a node, and all conduit segments connecting to it. */
  class DeleteNode : public QUndoCommand {
   public:
    explicit DeleteNode(diagram::FluirNode& node);

    void redo() override;
    void undo() override;

   private:
    ConduitCut cut_;
    RemovedElement removed_;
  };

}  // namespace fluir::editor

#endif
