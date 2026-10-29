#ifndef FLUIR_EDITOR_APP_MODULE_PAGE_HPP
#define FLUIR_EDITOR_APP_MODULE_PAGE_HPP

#include <filesystem>
#include <QWidget>

#include "editor/app/module_document.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/module_editor.hpp"
#include "editor/scene/graph_scene.hpp"
#include "editor/scene/graph_view.hpp"

namespace fluir::editor {

  /** A single open module. */
  class ModulePage : public QWidget {
    Q_OBJECT

   public:
    ModulePage(const EditorContext& ctx, ModuleEditor editor, QWidget* parent = nullptr);

    ModuleDocument& document() { return document_; }

    /** Saves the module to `program`, adds the .fl extension if it has none. */
    bool saveAs(std::filesystem::path program);

   signals:
    void closeRequested();

   private:
    void save();
    void saveAsDialog();

    ModuleDocument document_;
    GraphScene* scene_;
    GraphView* view_;
  };

}  // namespace fluir::editor

#endif
