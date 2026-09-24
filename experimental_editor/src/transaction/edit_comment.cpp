#include "editor/transaction/edit_comment.hpp"

#include <utility>

#include "editor/core/node_access.hpp"

namespace fluir::editor {

  bool EditCommentTransaction::execute(pt::ParseTree& tree) {
    pt::Comment* comment = commentAt(tree, path_);
    if (comment == nullptr || comment->text == text_) {
      return false;
    }
    std::swap(comment->text, text_);
    return true;
  }

}  // namespace fluir::editor
