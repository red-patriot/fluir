#ifndef FLUIR_EDITOR_TRANSACTIONS_ADD_RETURN_HPP
#define FLUIR_EDITOR_TRANSACTIONS_ADD_RETURN_HPP

#include <memory>
#include <utility>

#include "compiler/models/id.hpp"
#include "editor/core/tree.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Gives function `path` I32 return `newId`. Fails when it already has one. */
  class AddReturn : public Transaction {
   public:
    AddReturn(fluir::FullID path, fluir::ID newId) : path_(std::move(path)), id_(newId) { }

    bool execute(et::ParseTree& tree) override;
    bool unexecute(et::ParseTree& tree) override;

   private:
    fluir::FullID path_;
    fluir::ID id_;
    bool createdOutput_ = false;
  };

  std::unique_ptr<Transaction> addReturn(fluir::FullID path, fluir::ID newId);

}  // namespace fluir::editor

#endif
