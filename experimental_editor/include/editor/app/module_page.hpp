#ifndef FLUIR_EDITOR_APP_MODULE_PAGE_HPP
#define FLUIR_EDITOR_APP_MODULE_PAGE_HPP

#include <filesystem>
#include <memory>
#include <optional>
#include <QUndoStack>
#include <QWidget>

#include "editor/core/editor_context.hpp"
#include "editor/scene/graph_view.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor {

  /** A single open module. */
  class ModulePage : public QWidget {
    Q_OBJECT

   public:
    /** Shows `scene`. */
    ModulePage(const EditorContext& ctx,
               std::optional<std::filesystem::path> program,
               std::unique_ptr<ModuleScene> scene,
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
    QUndoStack* undoStack_;
    ModuleScene* scene_;
    GraphView* view_;
  };

}  // namespace fluir::editor

#endif
