#ifndef FLUIR_EDITOR_COMMANDS_ADD_PARAMETER_HPP
#define FLUIR_EDITOR_COMMANDS_ADD_PARAMETER_HPP

#include <optional>
#include <QUndoCommand>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/diagram/function.hpp"

namespace fluir::editor {

  /** Appends an I32 parameter named param<position + 1> to a function, with a fresh id. */
  class AddParameter : public QUndoCommand {
   public:
    explicit AddParameter(diagram::Function& function);

    void redo() override;
    void undo() override;

   private:
    diagram::Function& function_;
    bool hadInput_;
    pt::FunctionParameter data_;
    std::optional<diagram::Function::RemovedParameter> removed_;
  };

}  // namespace fluir::editor

#endif
