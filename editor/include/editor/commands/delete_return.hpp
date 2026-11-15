#ifndef FLUIR_EDITOR_COMMANDS_DELETE_RETURN_HPP
#define FLUIR_EDITOR_COMMANDS_DELETE_RETURN_HPP

#include <optional>
#include <QUndoCommand>

#include "editor/commands/conduit_cut.hpp"
#include "editor/diagram/function.hpp"

namespace fluir::editor {

  /** Removes a function's return and every conduit segment to it. */
  class DeleteReturn : public QUndoCommand {
   public:
    explicit DeleteReturn(diagram::Function& function);

    void redo() override;
    void undo() override;

   private:
    diagram::Function& function_;
    ConduitCut cut_;
    std::optional<diagram::Function::RemovedReturn> removed_;
  };

}  // namespace fluir::editor

#endif
