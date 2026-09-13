#ifndef FLUIR_EDITOR_TRANSACTION_UPDATE_FUNC_PARAM_HPP
#define FLUIR_EDITOR_TRANSACTION_UPDATE_FUNC_PARAM_HPP

#include <string>
#include <utility>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Renames a function's parameter with `Parameter::index` `index`, leaving its type alone. Self-inverting swap. */
  class UpdateFuncParamTransaction : public Transaction {
   public:
    UpdateFuncParamTransaction(fluir::FullID path, int index, std::string name) :
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
