#ifndef FLUIR_EDITOR_TRANSACTION_ADD_NODE_HPP
#define FLUIR_EDITOR_TRANSACTION_ADD_NODE_HPP

#include <memory>
#include <utility>
#include <variant>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/intelligence.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Adds operator or constant node `newId` at `location` into the body of function `parent`. */
  class AddNode : public Transaction {
   public:
    using Params = std::variant<OperatorOption, ConstantOption, CallFunctionOption, ConditionalOption>;

    AddNode(fluir::FullID parent, fluir::ID newId, fluir::FlowGraphLocation location, Params params) :
      parent_(std::move(parent)), id_(newId), location_(location), params_(std::move(params)) { }

    bool execute(pt::ParseTree& tree) override;
    bool unexecute(pt::ParseTree& tree) override;

   private:
    fluir::FullID parent_;
    fluir::ID id_;
    fluir::FlowGraphLocation location_;
    Params params_;
  };

  std::unique_ptr<Transaction> addNode(fluir::FullID parent,
                                       fluir::ID newId,
                                       fluir::FlowGraphLocation location,
                                       AddNode::Params params);

}  // namespace fluir::editor

#endif
