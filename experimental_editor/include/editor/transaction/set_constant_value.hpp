#ifndef FLUIR_EDITOR_TRANSACTION_SET_CONSTANT_VALUE_HPP
#define FLUIR_EDITOR_TRANSACTION_SET_CONSTANT_VALUE_HPP

#include <memory>
#include <utility>

#include "compiler/models/id.hpp"
#include "editor/core/tree.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Replaces a constant's literal, keeping its type. Self-inverting: the swap is its own reverse. */
  class SetConstantValueTransaction : public Transaction {
   public:
    SetConstantValueTransaction(fluir::FullID path, et::Literal value) :
      path_(std::move(path)), value_(std::move(value)) { }

    bool execute(et::ParseTree& tree) override;
    bool unexecute(et::ParseTree& tree) override { return execute(tree); }

   private:
    fluir::FullID path_;
    et::Literal value_;
  };

  std::unique_ptr<Transaction> setConstantValue(fluir::FullID path, et::Literal value);

}  // namespace fluir::editor

#endif
