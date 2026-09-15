#ifndef FLUIR_EDITOR_TRANSACTIONS_ADD_PARAMETER_HPP
#define FLUIR_EDITOR_TRANSACTIONS_ADD_PARAMETER_HPP

#include <utility>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Appends I32 parameter `newId` to function `path`, named `param{n}` after the last. */
  class AddParameter : public Transaction {
   public:
    AddParameter(fluir::FullID path, fluir::ID newId) : path_(std::move(path)), id_(newId) { }

    bool execute(pt::ParseTree& tree) override;
    bool unexecute(pt::ParseTree& tree) override;

   private:
    fluir::FullID path_;
    fluir::ID id_;
    bool createdInput_ = false;
  };

}  // namespace fluir::editor

#endif
