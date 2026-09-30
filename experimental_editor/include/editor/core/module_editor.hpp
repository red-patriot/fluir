#ifndef FLUIR_EDITOR_CORE_MODULE_EDITOR_HPP
#define FLUIR_EDITOR_CORE_MODULE_EDITOR_HPP

#include <deque>
#include <filesystem>
#include <memory>
#include <optional>

#include "editor/core/intelligence.hpp"
#include "editor/core/tree.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Owns the tree, the history of edits to it, and the Intelligence derived from it. Applying an edit forgets the
   * redo path. Intelligence is reloaded after every load, apply, record, undo and redo. */
  class ModuleEditor {
   public:
    /** Replaces the tree, forgets all history and loads Intelligence for `program`. */
    void load(std::optional<std::filesystem::path> program, et::ParseTree tree);

    /** Erases the program from Intelligence. */
    void unload();

    /** Moves the module to `program`. */
    void setProgram(std::filesystem::path program);

    const std::optional<std::filesystem::path>& program() const { return program_; }

    /** Executes `edit` and keeps it for undo. False when it changed nothing; it is then dropped. */
    bool apply(std::unique_ptr<Transaction> edit);

    /** Keeps an edit a live gesture already executed. Intelligence reflects the gesture only once it is recorded. */
    void record(std::unique_ptr<Transaction> edit);

    bool undo();
    bool redo();

    bool canUndo() const { return !undone_.empty(); }
    bool canRedo() const { return !redone_.empty(); }

    et::ParseTree& tree() { return tree_; }
    const et::ParseTree& tree() const { return tree_; }

    const Intelligence& intelligence() const { return intelligence_; }

    /** Generates a full ID that is unique, for inclusion in `body`. `body` may be empty, which generates a new
     * top-level ID.*/
    fluir::ID generateID(const fluir::FullID& body) const;

   private:
    void reload();

    std::optional<std::filesystem::path> program_;
    et::ParseTree tree_;
    Intelligence intelligence_;
    std::deque<std::unique_ptr<Transaction>> undone_;
    std::deque<std::unique_ptr<Transaction>> redone_;
  };

  /** An editor for the module at `program`. */
  std::optional<ModuleEditor> openModule(const std::filesystem::path& program);

  /** An editor for a new empty module. */
  ModuleEditor newModule();

}  // namespace fluir::editor

#endif
