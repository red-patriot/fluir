#ifndef FLUIR_EDITOR_TRANSACTIONS_ADD_PARAMETER_HPP
#define FLUIR_EDITOR_TRANSACTIONS_ADD_PARAMETER_HPP

#include <memory>
#include <utility>

#include "compiler/models/id.hpp"
#include "editor/core/tree.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Appends I32 parameter `newId` to function `path`, named `param{n}` after the last (skipping taken names). */
  class AddParameter : public Transaction {
   public:
    AddParameter(fluir::FullID path, fluir::ID newId) : path_(std::move(path)), id_(newId) { }

    bool execute(et::ParseTree& tree) override;
    bool unexecute(et::ParseTree& tree) override;

   private:
    fluir::FullID path_;
    fluir::ID id_;
    bool createdInput_ = false;
  };

  std::unique_ptr<Transaction> addParameter(fluir::FullID path, fluir::ID newId);

}  // namespace fluir::editor

#endif
