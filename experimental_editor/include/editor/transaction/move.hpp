#ifndef FLUIR_EDITOR_TRANSACTION_MOVE_HPP
#define FLUIR_EDITOR_TRANSACTION_MOVE_HPP

#include <utility>

#include "compiler/models/id.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Moves whatever `path` names to (x, y). Self-inverting: the swap is its own reverse. */
  class MoveTransaction : public Transaction {
   public:
    MoveTransaction(fluir::FullID path, int x, int y) : path_(std::move(path)), x_(x), y_(y) { }

    bool execute(pt::ParseTree& tree) override;
    bool unexecute(pt::ParseTree& tree) override { return execute(tree); }

   private:
    fluir::FullID path_;
    int x_;
    int y_;
  };

}  // namespace fluir::editor

#endif
