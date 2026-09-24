#include "editor/transaction/set_constant_value.hpp"

#include <memory>
#include <utility>
#include <variant>

#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  bool SetConstantValueTransaction::execute(pt::ParseTree& tree) {
    pt::Node* node = nodeAt(tree, path_);
    auto* constant = node == nullptr ? nullptr : std::get_if<pt::Constant>(node);
    if (constant == nullptr || constant->value.index() != value_.index() || constant->value == value_) {
      return false;
    }
    std::swap(constant->value, value_);
    return true;
  }

  std::unique_ptr<Transaction> setConstantValue(fluir::FullID path, pt::Literal value) {
    return std::make_unique<SetConstantValueTransaction>(std::move(path), std::move(value));
  }

}  // namespace fluir::editor
