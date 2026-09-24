#include "editor/transaction/edit_call_argument.hpp"

#include <algorithm>
#include <memory>
#include <utility>
#include <variant>

#include "editor/core/identifier.hpp"
#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  bool EditCallArgumentTransaction::execute(pt::ParseTree& tree) {
    pt::Node* node = nodeAt(tree, path_);
    auto* call = node == nullptr ? nullptr : std::get_if<pt::Call>(node);
    if (call == nullptr || !isValidIdentifier(name_)) {
      return false;
    }
    const auto arg = std::ranges::find(call->arguments, index_, &pt::Call::Argument::index);
    if (arg == call->arguments.end() || arg->name == name_) {
      return false;
    }
    std::swap(arg->name, name_);
    return true;
  }

  std::unique_ptr<Transaction> renameCallArgument(fluir::FullID path, int index, std::string name) {
    return std::make_unique<EditCallArgumentTransaction>(std::move(path), index, std::move(name));
  }

}  // namespace fluir::editor
