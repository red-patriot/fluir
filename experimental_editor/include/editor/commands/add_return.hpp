#ifndef FLUIR_EDITOR_COMMANDS_ADD_RETURN_HPP
#define FLUIR_EDITOR_COMMANDS_ADD_RETURN_HPP

#include <optional>
#include <QUndoCommand>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/diagram/function.hpp"

namespace fluir::editor {

  /** Adds an I32 return to a function that has none. */
  class AddReturn : public QUndoCommand {
   public:
    explicit AddReturn(diagram::Function& function);

    void redo() override;
    void undo() override;

   private:
    diagram::Function& function_;
    bool hadOutput_;
    pt::FunctionReturn data_;
    std::optional<diagram::Function::RemovedReturn> removed_;
  };

}  // namespace fluir::editor

#endif
