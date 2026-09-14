#include "editor/transaction/add_comment.hpp"

#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  bool AddComment::execute(pt::ParseTree& tree) {
    if (id_ == INVALID_ID) {
      return false;
    }
    const pt::Comment comment{id_, location_, ""};
    if (parent_.empty()) {
      return tree.declarations.emplace(id_, comment).second;
    }
    pt::Block* block = blockOf(tree, parent_);
    return block != nullptr && block->nodes.emplace(id_, comment).second;
  }

  bool AddComment::unexecute(pt::ParseTree& tree) {
    if (parent_.empty()) {
      return tree.declarations.erase(id_) > 0;
    }
    pt::Block* block = blockOf(tree, parent_);
    return block != nullptr && block->nodes.erase(id_) > 0;
  }

}  // namespace fluir::editor
