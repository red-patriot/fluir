#ifndef FLUIR_EDITOR_TRANSACTION_EDIT_COMMENT_HPP
#define FLUIR_EDITOR_TRANSACTION_EDIT_COMMENT_HPP

#include <string>
#include <utility>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Replaces a comment's text, top-level or in a body; any text is valid. Self-inverting swap. */
  class EditCommentTransaction : public Transaction {
   public:
    EditCommentTransaction(fluir::FullID path, std::string text) : path_(std::move(path)), text_(std::move(text)) { }

    bool execute(pt::ParseTree& tree) override;
    bool unexecute(pt::ParseTree& tree) override { return execute(tree); }

   private:
    fluir::FullID path_;
    std::string text_;
  };

}  // namespace fluir::editor

#endif
