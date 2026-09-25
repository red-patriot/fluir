#ifndef FLUIR_EDITOR_TRANSACTION_EDIT_CALL_NODE_HPP
#define FLUIR_EDITOR_TRANSACTION_EDIT_CALL_NODE_HPP

#include <memory>
#include <string>
#include <utility>

#include "compiler/models/id.hpp"
#include "editor/core/tree.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Changes which function a call targets, leaving arguments and return alone. Self-inverting swap. */
  class EditCallNodeTransaction : public Transaction {
   public:
    EditCallNodeTransaction(fluir::FullID path, std::string target) :
      path_(std::move(path)), target_(std::move(target)) { }

    bool execute(et::ParseTree& tree) override;
    bool unexecute(et::ParseTree& tree) override { return execute(tree); }

   private:
    fluir::FullID path_;
    std::string target_;
  };

  std::unique_ptr<Transaction> retargetCall(fluir::FullID path, std::string target);

}  // namespace fluir::editor

#endif
