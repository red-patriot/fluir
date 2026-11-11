#ifndef FLUIR_EDITOR_COMMANDS_ADD_CONDUIT_HPP
#define FLUIR_EDITOR_COMMANDS_ADD_CONDUIT_HPP

#include <memory>
#include <QUndoCommand>

#include "compiler/models/id.hpp"
#include "editor/commands/conduit_cut.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/scope.hpp"
#include "editor/diagram/terminal.hpp"

namespace fluir::editor {

  /** Wires `source` to `sink` in `scope` with a fresh id. */
  class AddConduit : public QUndoCommand {
   public:
    AddConduit(diagram::Scope& scope, diagram::Terminal& source, diagram::Terminal& sink);

    void redo() override;
    void undo() override;

   private:
    diagram::Scope& scope_;
    diagram::Terminal& source_;
    diagram::Terminal& sink_;
    fluir::ID id_;
    ConduitCut cut_;
    diagram::Conduit* conduit_ = nullptr;
    std::unique_ptr<diagram::Conduit> taken_;
  };

}  // namespace fluir::editor

#endif
