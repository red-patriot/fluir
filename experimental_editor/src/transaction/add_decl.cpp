#include "editor/transaction/add_decl.hpp"

#include <memory>
#include <optional>
#include <utility>

namespace fluir::editor {

  bool AddDecl::execute(et::ParseTree& tree) {
    if (!parent_.empty() || id_ == INVALID_ID || tree.declarations.contains(id_)) {
      return false;
    }
    // Duplicate names are left for the user to fix, as in the legacy editor.
    tree.declarations.emplace(id_, et::FunctionDecl{id_, location_, "new_function", {}, std::nullopt, std::nullopt});
    return true;
  }

  bool AddDecl::unexecute(et::ParseTree& tree) { return parent_.empty() && tree.declarations.erase(id_) > 0; }

  std::unique_ptr<Transaction> addDecl(fluir::FullID parent, fluir::ID newId, fluir::FlowGraphLocation location) {
    return std::make_unique<AddDecl>(std::move(parent), newId, location);
  }

}  // namespace fluir::editor
