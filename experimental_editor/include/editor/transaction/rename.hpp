#ifndef FLUIR_EDITOR_TRANSACTION_RENAME_HPP
#define FLUIR_EDITOR_TRANSACTION_RENAME_HPP

#include <memory>
#include <string>
#include <utility>

#include "compiler/models/id.hpp"
#include "editor/core/tree.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Renames a function decl; calls to the old name are not retargeted. Self-inverting swap. */
  class RenameTransaction : public Transaction {
   public:
    RenameTransaction(fluir::FullID path, std::string name) : path_(std::move(path)), name_(std::move(name)) { }

    bool execute(et::ParseTree& tree) override;
    bool unexecute(et::ParseTree& tree) override { return execute(tree); }

   private:
    fluir::FullID path_;
    std::string name_;
  };

  std::unique_ptr<Transaction> renameFunction(fluir::FullID path, std::string name);

}  // namespace fluir::editor

#endif
