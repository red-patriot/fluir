#ifndef FLUIR_EDITOR_COMMANDS_SET_FIELD_HPP
#define FLUIR_EDITOR_COMMANDS_SET_FIELD_HPP

#include <QUndoCommand>
#include <string>
#include <utility>

#include "editor/core/field.hpp"
#include "editor/diagram/node.hpp"

namespace fluir::editor {

  /** Sets one of a node's fields from `before` to `after`; both must be text the node accepts. */
  class SetField : public QUndoCommand {
   public:
    SetField(diagram::FluirNode& node, Field field, std::string before, std::string after) :
      node_(node), field_(field), before_(std::move(before)), after_(std::move(after)) { }

    void redo() override { node_.setText(field_, after_); }
    void undo() override { node_.setText(field_, before_); }

   private:
    diagram::FluirNode& node_;
    Field field_;
    std::string before_;
    std::string after_;
  };

}  // namespace fluir::editor

#endif
