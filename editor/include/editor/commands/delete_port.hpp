#ifndef FLUIR_EDITOR_COMMANDS_DELETE_PORT_HPP
#define FLUIR_EDITOR_COMMANDS_DELETE_PORT_HPP

#include <optional>
#include <QUndoCommand>

#include "editor/commands/conduit_cut.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/view/draw/conditional.hpp"

namespace fluir::editor {

  /** Removes a conditional's port and every conduit segment on it. */
  class DeletePort : public QUndoCommand {
   public:
    DeletePort(diagram::Conditional& conditional, draw::PortRef ref);

    void redo() override;
    void undo() override;

   private:
    diagram::Conditional& conditional_;
    draw::PortRef ref_;
    ConduitCut cut_;
    std::optional<diagram::Conditional::RemovedPort> removed_;
  };

}  // namespace fluir::editor

#endif
