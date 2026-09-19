#ifndef FLUIR_EDITOR_TRANSACTION_EDIT_CALL_ARGUMENT_HPP
#define FLUIR_EDITOR_TRANSACTION_EDIT_CALL_ARGUMENT_HPP

#include <string>
#include <utility>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Renames a call's argument with `Argument::index` `index`, leaving target and return alone. Self-inverting swap. */
  class EditCallArgumentTransaction : public Transaction {
   public:
    EditCallArgumentTransaction(fluir::FullID path, int index, std::string name) :
      path_(std::move(path)), index_(index), name_(std::move(name)) { }

    bool execute(pt::ParseTree& tree) override;
    bool unexecute(pt::ParseTree& tree) override { return execute(tree); }

   private:
    fluir::FullID path_;
    int index_;
    std::string name_;
  };

}  // namespace fluir::editor

#endif
