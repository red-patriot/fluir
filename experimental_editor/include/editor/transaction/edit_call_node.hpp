#ifndef FLUIR_EDITOR_TRANSACTION_EDIT_CALL_NODE_HPP
#define FLUIR_EDITOR_TRANSACTION_EDIT_CALL_NODE_HPP

#include <string>
#include <utility>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Changes which function a call targets, leaving arguments and return alone. Self-inverting swap. */
  class EditCallNodeTransaction : public Transaction {
   public:
    EditCallNodeTransaction(fluir::FullID path, std::string target) :
      path_(std::move(path)), target_(std::move(target)) { }

    bool execute(pt::ParseTree& tree) override;
    bool unexecute(pt::ParseTree& tree) override { return execute(tree); }

   private:
    fluir::FullID path_;
    std::string target_;
  };

}  // namespace fluir::editor

#endif
