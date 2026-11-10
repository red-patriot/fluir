#ifndef FLUIR_EDITOR_COMMANDS_PLACE_NODE_HPP
#define FLUIR_EDITOR_COMMANDS_PLACE_NODE_HPP

#include <QUndoCommand>

#include "compiler/models/location.hpp"
#include "editor/diagram/node.hpp"

namespace fluir::editor {

  /** Moves or sizes a node from `before` to `after`. */
  class PlaceNode : public QUndoCommand {
   public:
    PlaceNode(diagram::FluirNode& node, FlowGraphLocation before, FlowGraphLocation after) :
      node_(node), before_(before), after_(after) { }

    void redo() override { node_.setLocation(after_); }
    void undo() override { node_.setLocation(before_); }

   private:
    diagram::FluirNode& node_;
    FlowGraphLocation before_;
    FlowGraphLocation after_;
  };

}  // namespace fluir::editor

#endif
