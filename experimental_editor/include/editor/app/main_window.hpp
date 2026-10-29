#ifndef FLUIR_EDITOR_APP_MAIN_WINDOW_HPP
#define FLUIR_EDITOR_APP_MAIN_WINDOW_HPP

#include <filesystem>
#include <QMainWindow>
#include <QStackedWidget>

#include "editor/app/module_document.hpp"
#include "editor/app/module_page.hpp"
#include "editor/app/splash_widget.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/module_editor.hpp"

namespace fluir::editor {

  /** The editor window. */
  class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    explicit MainWindow(const EditorContext& ctx, QWidget* parent = nullptr);

    /** The open module, or null on the splash. */
    ModuleDocument* document() const { return page_ ? &page_->document() : nullptr; }

    /** Opens the module at `program`.
     * \return true when the module loaded successfully, false when it fails to load. */
    bool openModule(const std::filesystem::path& program);

   private:
    void openPage(ModuleEditor editor);
    void closePage();
    void openDialog();
    void retitle();

    const EditorContext& ctx_;
    QStackedWidget* pages_;
    SplashWidget* splash_;
    ModulePage* page_ = nullptr;
  };

}  // namespace fluir::editor

#endif
