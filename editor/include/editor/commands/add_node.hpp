#ifndef FLUIR_EDITOR_COMMANDS_ADD_NODE_HPP
#define FLUIR_EDITOR_COMMANDS_ADD_NODE_HPP

#include <QUndoCommand>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/commands/removed_element.hpp"
#include "editor/core/geometry.hpp"
#include "editor/diagram/scope.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor {

  /** Adds a node made from a prototype at grid point `at`. */
  class AddNode : public QUndoCommand {
   public:
    AddNode(diagram::Scope& scope, const pt::Node& prototype, Vec2i at);
    AddNode(ModuleScene& scene, const pt::Declaration& prototype, Vec2i at);

    void redo() override;
    void undo() override;

   private:
    RemovedElement removed_;
  };

}  // namespace fluir::editor

#endif
