#include "editor/transaction/rename.hpp"

#include <utility>

#include "editor/core/identifier.hpp"
#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  bool RenameTransaction::execute(pt::ParseTree& tree) {
    pt::FunctionDecl* fn = functionAt(tree, path_);
    if (fn == nullptr || !isValidIdentifier(name_) || fn->name == name_) {
      return false;
    }
    std::swap(fn->name, name_);
    return true;
  }

}  // namespace fluir::editor
