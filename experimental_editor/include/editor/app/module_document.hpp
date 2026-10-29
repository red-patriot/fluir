#ifndef FLUIR_EDITOR_APP_MODULE_DOCUMENT_HPP
#define FLUIR_EDITOR_APP_MODULE_DOCUMENT_HPP

#include <filesystem>
#include <functional>
#include <memory>
#include <QObject>

#include "editor/core/module_editor.hpp"
#include "editor/core/tree.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** An open module. */
  class ModuleDocument : public QObject {
    Q_OBJECT

   public:
    explicit ModuleDocument(ModuleEditor editor, QObject* parent = nullptr);
    ~ModuleDocument() override;

    const ModuleEditor& editor() const { return editor_; }

    bool apply(std::unique_ptr<Transaction> edit);
    void record(std::unique_ptr<Transaction> edit);
    bool undo();
    bool redo();

    /** Runs `change` on the tree outside the history, for view-only annotations such as the shown branch. */
    void touch(const std::function<void(et::ParseTree&)>& change);

    /** Writes the tree to its program.
     * \return true when the write succeeds, false when it has no program or the write fails. */
    bool save();
    /** Writes the tree to `program` and moves the module there.
     * \return true when the write succeeds, false when the write fails. */
    bool saveAs(const std::filesystem::path& program);

   signals:
    void changed();
    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);
    void programChanged();

   private:
    void announce();

    ModuleEditor editor_;
    bool canUndo_;
    bool canRedo_;
  };

}  // namespace fluir::editor

#endif
