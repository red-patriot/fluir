#include "editor/transaction/edit_comment.hpp"

#include <utility>
#include <variant>

#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  bool EditCommentTransaction::execute(pt::ParseTree& tree) {
    pt::Comment* comment = nullptr;
    if (path_.size() == 1) {
      pt::Declaration* decl = declarationAt(tree, path_);
      comment = decl == nullptr ? nullptr : std::get_if<pt::Comment>(decl);
    } else {
      pt::Node* node = nodeAt(tree, path_);
      comment = node == nullptr ? nullptr : std::get_if<pt::Comment>(node);
    }
    if (comment == nullptr || comment->text == text_) {
      return false;
    }
    std::swap(comment->text, text_);
    return true;
  }

}  // namespace fluir::editor
