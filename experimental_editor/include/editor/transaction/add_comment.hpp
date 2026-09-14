#ifndef FLUIR_EDITOR_TRANSACTION_ADD_COMMENT_HPP
#define FLUIR_EDITOR_TRANSACTION_ADD_COMMENT_HPP

#include <utility>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Adds an empty comment `newId` at `location`: top-level for an empty `parent`, else into its block. */
  class AddComment : public Transaction {
   public:
    AddComment(fluir::FullID parent, fluir::ID newId, fluir::FlowGraphLocation location) :
      parent_(std::move(parent)), id_(newId), location_(location) { }

    bool execute(pt::ParseTree& tree) override;
    bool unexecute(pt::ParseTree& tree) override;

   private:
    fluir::FullID parent_;
    fluir::ID id_;
    fluir::FlowGraphLocation location_;
  };

}  // namespace fluir::editor

#endif
