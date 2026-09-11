#ifndef FLUIR_EDITOR_TRANSACTION_MOVE_HPP
#define FLUIR_EDITOR_TRANSACTION_MOVE_HPP

#include <utility>

#include "compiler/models/id.hpp"
#include "editor/actors/scene.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Moves an actor to (x, y). Self-inverting: the swap is its own reverse. */
  class MoveTransaction : public Transaction {
   public:
    MoveTransaction(fluir::FullID id, int x, int y) : id_(std::move(id)), x_(x), y_(y) { }

    bool execute(GraphScene& scene) override;
    bool unexecute(GraphScene& scene) override { return execute(scene); }

   private:
    fluir::FullID id_;
    int x_;
    int y_;
  };

}  // namespace fluir::editor

#endif
