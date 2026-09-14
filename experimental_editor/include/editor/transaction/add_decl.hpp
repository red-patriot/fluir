#ifndef FLUIR_EDITOR_TRANSACTIONS_ADD_DECL_HPP
#define FLUIR_EDITOR_TRANSACTIONS_ADD_DECL_HPP

#include <utility>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Adds an empty top-level function `newId` at `location`. Only top-level `parent` is valid. */
  class AddDecl : public Transaction {
   public:
    AddDecl(fluir::FullID parent, fluir::ID newId, fluir::FlowGraphLocation location) :
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
