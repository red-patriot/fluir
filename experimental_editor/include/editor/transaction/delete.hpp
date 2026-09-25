#ifndef FLUIR_EDITOR_TRANSACTION_DELETE_HPP
#define FLUIR_EDITOR_TRANSACTION_DELETE_HPP

#include <cstddef>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "compiler/models/id.hpp"
#include "editor/core/tree.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Deletes a top-level declaration, a node, or a function's parameter or return rail, with every reference to it.
   *  Keeps only what it touched. */
  class DeleteTransaction : public Transaction {
   public:
    explicit DeleteTransaction(fluir::FullID path) : path_(std::move(path)) { }

    bool execute(et::ParseTree& tree) override;
    bool unexecute(et::ParseTree& tree) override;

   private:
    bool executeRail(et::ParseTree& tree);

    fluir::FullID path_;
    std::optional<et::Declaration> declaration_;
    std::optional<et::Node> node_;
    std::optional<et::FunctionDecl::Parameter> param_;
    std::size_t paramPos_ = 0; /**< where `param_` sat in its parameter list */
    std::optional<et::FunctionDecl::Return> ret_;
    std::vector<et::Conduit> conduits_; /**< touched conduits, as they were */
    std::vector<et::Node> referrers_;   /**< nodes whose operands named it, as they were */
  };

  std::unique_ptr<Transaction> deleteAt(fluir::FullID path);

}  // namespace fluir::editor

#endif
