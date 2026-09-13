#ifndef FLUIR_EDITOR_TRANSACTION_DELETE_HPP
#define FLUIR_EDITOR_TRANSACTION_DELETE_HPP

#include <optional>
#include <utility>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Deletes a function, or a node and every reference to it. Keeps only what it touched. */
  class DeleteTransaction : public Transaction {
   public:
    explicit DeleteTransaction(fluir::FullID path) : path_(std::move(path)) { }

    bool execute(pt::ParseTree& tree) override;
    bool unexecute(pt::ParseTree& tree) override;

   private:
    fluir::FullID path_;
    std::optional<pt::Declaration> function_;
    std::optional<pt::Node> node_;
    std::vector<pt::Conduit> conduits_; /**< touched conduits, as they were */
    std::vector<pt::Node> referrers_;   /**< nodes whose operands named it, as they were */
  };

}  // namespace fluir::editor

#endif
