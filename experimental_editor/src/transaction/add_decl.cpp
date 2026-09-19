#include "editor/transaction/add_decl.hpp"

#include <optional>

namespace fluir::editor {

  bool AddDecl::execute(pt::ParseTree& tree) {
    if (!parent_.empty() || id_ == INVALID_ID || tree.declarations.contains(id_)) {
      return false;
    }
    // Duplicate names are left for the user to fix, as in the legacy editor.
    tree.declarations.emplace(id_, pt::FunctionDecl{id_, location_, "new_function", {}, std::nullopt, std::nullopt});
    return true;
  }

  bool AddDecl::unexecute(pt::ParseTree& tree) { return parent_.empty() && tree.declarations.erase(id_) > 0; }

}  // namespace fluir::editor
