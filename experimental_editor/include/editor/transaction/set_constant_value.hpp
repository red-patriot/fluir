#ifndef FLUIR_EDITOR_TRANSACTION_SET_CONSTANT_VALUE_HPP
#define FLUIR_EDITOR_TRANSACTION_SET_CONSTANT_VALUE_HPP

#include <utility>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/actors/scene.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Replaces a constant's literal. Self-inverting: the swap is its own reverse. */
  class SetConstantValueTransaction : public Transaction {
   public:
    SetConstantValueTransaction(fluir::FullID id, pt::Literal value) : id_(std::move(id)), value_(std::move(value)) { }

    bool execute(GraphScene& scene) override;
    bool unexecute(GraphScene& scene) override { return execute(scene); }

   private:
    fluir::FullID id_;
    pt::Literal value_;
  };

}  // namespace fluir::editor

#endif
