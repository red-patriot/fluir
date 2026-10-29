#ifndef FLUIR_EDITOR_APP_MAIN_WINDOW_HPP
#define FLUIR_EDITOR_APP_MAIN_WINDOW_HPP

#include <filesystem>
#include <memory>
#include <QMainWindow>

#include "editor/app/module_document.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/module_editor.hpp"

// @CLAUDE: why not just include the headers? Seems better I think.
class QAction;
class QStackedWidget;
class QToolBar;

namespace fluir::editor {

  // @CLAUDE: Ditto about including rather than forward declaring
  class GraphView;
  class SplashWidget;

  /** The editor window: the splash until a module is open, then the module's graph under a toolbar. */
  class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    explicit MainWindow(const EditorContext& ctx, QWidget* parent = nullptr);
    ~MainWindow() override;

    /** The open module, or null on the splash. */
    ModuleDocument* document() const { return document_.get(); }

    /** Opens the module at `program`.
     * \return true when the module loaded successfully, false when it fails to load. */
    bool openModule(const std::filesystem::path& program);
    /** Saves the open module to `program`, adds the .fl extension when it has none. */
    bool saveModuleAs(std::filesystem::path program);

   private:
    void openEditor(ModuleEditor editor);
    void closeModule();
    void openDialog();
    void save();
    void saveAsDialog();
    void refresh();

    std::unique_ptr<ModuleDocument> document_;
    QStackedWidget* pages_;
    SplashWidget* splash_;
    GraphView* graph_;
    QToolBar* toolbar_;
    // @CLAUDE: A lot of these seem to be related to the module page, not the general window. COnsider putting them
    // there instead? Also, why do we need all these? Can't qt store this internally for us, instead of us manually
    // saving all them? Seems to overcomplicate things.
    QAction* save_;
    QAction* saveAs_;
    QAction* undo_;
    QAction* redo_;
    QAction* exit_;
  };

}  // namespace fluir::editor

#endif
