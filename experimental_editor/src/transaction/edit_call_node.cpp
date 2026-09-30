#include "editor/transaction/edit_call_node.hpp"

#include <memory>
#include <utility>
#include <variant>

#include "editor/core/identifier.hpp"
#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  bool EditCallNodeTransaction::execute(et::ParseTree& tree) {
    et::Node* node = nodeAt(tree, path_);
    auto* call = node == nullptr ? nullptr : std::get_if<et::Call>(node);
    if (call == nullptr || !isValidIdentifier(target_) || call->target == target_) {
      return false;
    }
    std::swap(call->target, target_);
    return true;
  }

  std::unique_ptr<Transaction> retargetCall(fluir::FullID path, std::string target) {
    return std::make_unique<EditCallNodeTransaction>(std::move(path), std::move(target));
  }

}  // namespace fluir::editor
