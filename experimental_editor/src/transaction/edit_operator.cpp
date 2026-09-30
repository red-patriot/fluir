#include "editor/transaction/edit_operator.hpp"

#include <memory>
#include <utility>
#include <variant>

#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  bool EditOperatorTransaction::execute(et::ParseTree& tree) {
    et::Node* node = nodeAt(tree, path_);
    fluir::Operator* op = nullptr;
    if (auto* binary = std::get_if<et::Binary>(node)) {
      op = &binary->op;
    } else if (auto* unary = std::get_if<et::Unary>(node)) {
      op = &unary->op;
    }
    if (op == nullptr || op_ == fluir::Operator::UNKNOWN || *op == op_) {
      return false;
    }
    std::swap(*op, op_);
    return true;
  }

  std::unique_ptr<Transaction> setOperator(fluir::FullID path, fluir::Operator op) {
    return std::make_unique<EditOperatorTransaction>(std::move(path), op);
  }

}  // namespace fluir::editor
