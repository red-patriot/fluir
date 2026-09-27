#ifndef FLUIR_EDITOR_TRANSACTION_MOVE_PORT_HPP
#define FLUIR_EDITOR_TRANSACTION_MOVE_PORT_HPP

#include <memory>
#include <utility>

#include "compiler/models/id.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Moves a Block `port` at `path` to `y`. */
  class MovePortTransaction : public Transaction {
   public:
    MovePortTransaction(fluir::FullID path, PortRef port, int y) : path_(std::move(path)), port_(port), y_(y) { }

    bool execute(et::ParseTree& tree) override;
    bool unexecute(et::ParseTree& tree) override { return execute(tree); }

   private:
    fluir::FullID path_;
    PortRef port_;
    int y_;
  };

  std::unique_ptr<Transaction> movePortTo(fluir::FullID path, PortRef port, int y);

}  // namespace fluir::editor

#endif
