#ifndef FLUIR_EDITOR_TRANSACTION_EDIT_OPERATOR_HPP
#define FLUIR_EDITOR_TRANSACTION_EDIT_OPERATOR_HPP

#include <utility>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/operator.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Changes the operator of a binary or unary node, leaving operands alone. Self-inverting swap. */
  class EditOperatorTransaction : public Transaction {
   public:
    EditOperatorTransaction(fluir::FullID path, fluir::Operator op) : path_(std::move(path)), op_(op) { }

    bool execute(pt::ParseTree& tree) override;
    bool unexecute(pt::ParseTree& tree) override { return execute(tree); }

   private:
    fluir::FullID path_;
    fluir::Operator op_;
  };

}  // namespace fluir::editor

#endif
