#ifndef FLUIR_EDITOR_COMMANDS_DELETE_PARAMETER_HPP
#define FLUIR_EDITOR_COMMANDS_DELETE_PARAMETER_HPP

#include <optional>
#include <QUndoCommand>

#include "compiler/models/id.hpp"
#include "editor/commands/conduit_cut.hpp"
#include "editor/diagram/function.hpp"

namespace fluir::editor {

  /** Removes a function's parameter and every conduit from it. */
  class DeleteParameter : public QUndoCommand {
   public:
    DeleteParameter(diagram::Function& function, fluir::ID id);

    void redo() override;
    void undo() override;

   private:
    diagram::Function& function_;
    fluir::ID id_;
    ConduitCut cut_;
    std::optional<diagram::Function::RemovedParameter> removed_;
  };

}  // namespace fluir::editor

#endif
