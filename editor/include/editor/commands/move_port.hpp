#ifndef FLUIR_EDITOR_COMMANDS_MOVE_PORT_HPP
#define FLUIR_EDITOR_COMMANDS_MOVE_PORT_HPP

#include <QUndoCommand>

#include "editor/diagram/conditional.hpp"
#include "editor/view/draw/conditional.hpp"

namespace fluir::editor {

  /** Slides a conditional's port along its wall from `before` to `after`, in grid units. */
  class MovePort : public QUndoCommand {
   public:
    MovePort(diagram::Conditional& conditional, draw::PortRef port, int before, int after) :
      conditional_(conditional), port_(port), before_(before), after_(after) { }

    void redo() override { conditional_.setPortY(port_, after_); }
    void undo() override { conditional_.setPortY(port_, before_); }

   private:
    diagram::Conditional& conditional_;
    draw::PortRef port_;
    int before_;
    int after_;
  };

}  // namespace fluir::editor

#endif
