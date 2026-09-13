#ifndef FLUIR_EDITOR_TRANSACTION_TRANSACTION_HPP
#define FLUIR_EDITOR_TRANSACTION_TRANSACTION_HPP

#include "compiler/frontend/parse_tree/parse_tree.hpp"

namespace fluir::editor {

  /** A reversible edit to a tree. Redo re-calls `execute`, so `execute` must be
   *  valid from the state `unexecute` leaves behind. */
  class Transaction {
   public:
    virtual ~Transaction() = default;

    /** Applies the edit. False when nothing changed. */
    virtual bool execute(pt::ParseTree& tree) = 0;
    /** Reverses `execute`. False when the tree is not in the expected state. */
    virtual bool unexecute(pt::ParseTree& tree) = 0;
  };

}  // namespace fluir::editor

#endif
