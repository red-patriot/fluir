#pragma once

#include <deque>
#include <memory>

#include "editor/actors/scene.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Owns the scene and the history of edits to it. Applying an edit forgets the redo path. */
  class ModuleEditor {
   public:
    /** Executes `edit` and keeps it for undo. False when it changed nothing; it is then dropped. */
    bool apply(std::unique_ptr<Transaction> edit);
    bool undo();
    bool redo();

    bool canUndo() const { return !undone_.empty(); }
    bool canRedo() const { return !redone_.empty(); }

    /** Empties the scene and forgets all history, as loading a new module must. */
    void reset();

    GraphScene& scene() { return scene_; }
    const GraphScene& scene() const { return scene_; }

   private:
    GraphScene scene_;
    std::deque<std::unique_ptr<Transaction>> undone_;
    std::deque<std::unique_ptr<Transaction>> redone_;
  };

}  // namespace fluir::editor
