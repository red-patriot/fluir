#ifndef FLUIR_EDITOR_APP_MODULE_PAGE_HPP
#define FLUIR_EDITOR_APP_MODULE_PAGE_HPP

#include <filesystem>
#include <optional>
#include <QUndoStack>
#include <QWidget>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/intelligence.hpp"
#include "editor/scene/graph_view.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor {

  /** A single open module. */
  class ModulePage : public QWidget {
    Q_OBJECT

   public:
    /** Shows `tree`'s diagram. Throws std::invalid_argument if `tree` describes no valid diagram. */
    ModulePage(const EditorContext& ctx,
               std::optional<std::filesystem::path> program,
               const pt::ParseTree& tree,
               QWidget* parent = nullptr);

    const std::optional<std::filesystem::path>& program() const { return program_; }

    QUndoStack& undoStack() { return *undoStack_; }

    /** Saves the module to `program`, adds the .fl extension if it has none. */
    bool saveAs(std::filesystem::path program);

   signals:
    void closeRequested();
    void programChanged();

   private:
    void save();
    void saveAsDialog();
    bool write(const std::filesystem::path& program);

    std::optional<std::filesystem::path> program_;
    Intelligence intelligence_;
    QUndoStack* undoStack_;
    ModuleScene* scene_;
    GraphView* view_;
  };

}  // namespace fluir::editor

#endif
