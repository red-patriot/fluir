#ifndef FLUIR_EDITOR_APP_MAIN_WINDOW_HPP
#define FLUIR_EDITOR_APP_MAIN_WINDOW_HPP

#include <filesystem>
#include <optional>
#include <QMainWindow>
#include <QStackedWidget>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/app/module_page.hpp"
#include "editor/app/splash_widget.hpp"
#include "editor/core/editor_context.hpp"

namespace fluir::editor {

  /** The editor window. */
  class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    explicit MainWindow(const EditorContext& ctx, QWidget* parent = nullptr);

    /** Opens the module at `program`.
     * \return true when the module loaded successfully, false when it fails to load. */
    bool openModule(const std::filesystem::path& program);

   private:
    void openPage(std::optional<std::filesystem::path> program, const pt::ParseTree& tree);
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
