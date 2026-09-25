#pragma once

#include <deque>
#include <memory>

#include "editor/core/tree.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Owns the tree and the history of edits to it. Applying an edit forgets the redo path. */
  class ModuleEditor {
   public:
    /** Replaces the tree and forgets all history. */
    void load(et::ParseTree tree);

    /** Executes `edit` and keeps it for undo. False when it changed nothing; it is then dropped. */
    bool apply(std::unique_ptr<Transaction> edit);

    /** Keeps an edit a live gesture already executed. */
    void record(std::unique_ptr<Transaction> edit);

    bool undo();
    bool redo();

    bool canUndo() const { return !undone_.empty(); }
    bool canRedo() const { return !redone_.empty(); }

    et::ParseTree& tree() { return tree_; }
    const et::ParseTree& tree() const { return tree_; }

    /** Generates a full ID that is unique, for inclusion in `body`. `body` may be empty, which generates a new
     * top-level ID.*/
    fluir::ID generateID(const fluir::FullID& body) const;

   private:
    et::ParseTree tree_;
    std::deque<std::unique_ptr<Transaction>> undone_;
    std::deque<std::unique_ptr<Transaction>> redone_;
  };

}  // namespace fluir::editor
