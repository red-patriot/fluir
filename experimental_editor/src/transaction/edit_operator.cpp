#include "editor/transaction/edit_operator.hpp"

#include <utility>
#include <variant>

#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  bool EditOperatorTransaction::execute(pt::ParseTree& tree) {
    pt::Node* node = nodeAt(tree, path_);
    fluir::Operator* op = nullptr;
    if (auto* binary = std::get_if<pt::Binary>(node)) {
      op = &binary->op;
    } else if (auto* unary = std::get_if<pt::Unary>(node)) {
      op = &unary->op;
    }
    if (op == nullptr || op_ == fluir::Operator::UNKNOWN || *op == op_) {
      return false;
    }
    std::swap(*op, op_);
    return true;
  }

}  // namespace fluir::editor
