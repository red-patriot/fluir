#ifndef FLUIR_EDITOR_TRANSACTION_DELETE_NODE_HPP
#define FLUIR_EDITOR_TRANSACTION_DELETE_NODE_HPP

#include <utility>
#include <vector>

#include "compiler/models/id.hpp"
#include "editor/actors/scene.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Deletes one node and every conduit wired to it, as the shipped delete does. */
  class DeleteNodeTransaction : public Transaction {
   public:
    /** An operand reset by this delete: the node naming it, and which slot. */
    struct ClearedOperand {
      fluir::FullID node;
      int slot; /**< 0 = lhs, 1 = rhs */
    };

    explicit DeleteNodeTransaction(fluir::FullID id) : id_(std::move(id)) { }

    bool execute(GraphScene& scene) override;
    bool unexecute(GraphScene& scene) override;

   private:
    fluir::FullID id_;
    std::vector<GraphScene::DetachedActor> removed_; /**< detach order; undone last-first */
    std::vector<ClearedOperand> clearedOperands_;
  };

}  // namespace fluir::editor

#endif
