#ifndef FLUIR_EDITOR_COMMANDS_ADD_PORT_HPP
#define FLUIR_EDITOR_COMMANDS_ADD_PORT_HPP

#include <optional>
#include <QUndoCommand>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/view/draw/conditional.hpp"

namespace fluir::editor {

  /** Adds a port to a conditional at its wall's middle. */
  class AddPort : public QUndoCommand {
   public:
    AddPort(diagram::Conditional& conditional, bool output);

    void redo() override;
    void undo() override;

   private:
    diagram::Conditional& conditional_;
    bool output_;
    pt::BlockPort data_;
    draw::PortRef ref_;
    std::optional<diagram::Conditional::RemovedPort> removed_;
  };

}  // namespace fluir::editor

#endif
