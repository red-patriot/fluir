#ifndef FLUIR_EDITOR_TRANSACTION_UPDATE_FUNC_PARAM_HPP
#define FLUIR_EDITOR_TRANSACTION_UPDATE_FUNC_PARAM_HPP

#include <memory>
#include <string>
#include <utility>
#include <variant>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Renames a function's parameter or sets a param's or return's type. Self-inverting swap. */
  class UpdateFuncParamTransaction : public Transaction {
   public:
    /** Renames the parameter with `Parameter::index` `index`, leaving its type alone. */
    static std::unique_ptr<UpdateFuncParamTransaction> rename(fluir::FullID path, int index, std::string name);
    /** Sets the type of the param or return with id `railId`. */
    static std::unique_ptr<UpdateFuncParamTransaction> setType(fluir::FullID path,
                                                               fluir::ID railId,
                                                               std::string typeName);

    bool execute(pt::ParseTree& tree) override;
    bool unexecute(pt::ParseTree& tree) override { return execute(tree); }

   private:
    struct Rename {
      int index;
      std::string name;
    };
    struct SetType {
      fluir::ID railId;
      std::string typeName;
    };

    UpdateFuncParamTransaction(fluir::FullID path, std::variant<Rename, SetType> edit) :
      path_(std::move(path)), edit_(std::move(edit)) { }

    fluir::FullID path_;
    std::variant<Rename, SetType> edit_;
  };

  std::unique_ptr<Transaction> renameParameter(fluir::FullID path, int index, std::string name);
  std::unique_ptr<Transaction> setRailType(fluir::FullID path, fluir::ID railId, std::string typeName);

}  // namespace fluir::editor

#endif
