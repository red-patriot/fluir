#include "editor/app/main_window.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <QFileDialog>
#include <QMessageBox>
#include <QStackedWidget>
#include <utility>

#include "bytecode/version.hpp"
#include "compiler/utility/context.hpp"
#include "editor/core/loader.hpp"
#include "editor/diagram/build.hpp"

namespace fluir::editor {
  namespace {

    std::unique_ptr<ModuleScene> newModule(const EditorContext& ctx) {
      auto scene = std::make_unique<ModuleScene>(ctx);
      scene->setHeader({.version = fluir::CURRENT_VERSION});
      return scene;
    }

  }  // namespace

  MainWindow::MainWindow(const EditorContext& ctx, QWidget* parent) :
    QMainWindow(parent), ctx_(ctx), pages_(new QStackedWidget(this)), splash_(new SplashWidget(pages_)) {
    pages_->addWidget(splash_);
    setCentralWidget(pages_);
    resize(ctx.window.width, ctx.window.height);

    connect(splash_, &SplashWidget::newRequested, this, [this] { openPage(std::nullopt, newModule(ctx_)); });
    connect(splash_, &SplashWidget::openRequested, this, &MainWindow::openDialog);

    retitle();
  }

  bool MainWindow::openModule(const std::filesystem::path& program) {
    fluir::Context ctx{.currentFile = program, .ignoreVersionChecks = true};
    LoadResult result = loadFile(ctx, program);
    if (!result.tree) {
      return false;
    }
    auto scene = std::make_unique<ModuleScene>(ctx_);
    if (!diagram::buildScene(*result.tree, *scene)) {
      return false;
    }
    openPage(program, std::move(scene));
    return true;
  }

  void MainWindow::openPage(std::optional<std::filesystem::path> program, std::unique_ptr<ModuleScene> scene) {
    closePage();
    page_ = new ModulePage(ctx_, std::move(program), std::move(scene), pages_);
    pages_->addWidget(page_);
    pages_->setCurrentWidget(page_);
    connect(page_, &ModulePage::closeRequested, this, &MainWindow::closePage);
    connect(page_, &ModulePage::programChanged, this, &MainWindow::retitle);
    retitle();
  }

  // Deferred: the page's own exit action may be the caller.
  void MainWindow::closePage() {
    if (page_ == nullptr) {
      return;
    }
    pages_->removeWidget(page_);
    page_->deleteLater();
    page_ = nullptr;
    retitle();
  }

  void MainWindow::openDialog() {
    const QString chosen = QFileDialog::getOpenFileName(this, tr("Open"), {}, tr("Fluir Program (*.fl)"));
    if (!chosen.isEmpty() && !openModule(chosen.toStdU16String())) {
      QMessageBox::warning(this, tr("Open"), tr("%1 failed to load.").arg(chosen));
    }
  }

  void MainWindow::retitle() {
    QString title = tr("Fluir");
    if (page_ != nullptr) {
      const auto& program = page_->program();
      title += " - " + (program ? QString::fromStdU16String(program->filename().u16string()) : tr("<new file>"));
    }
    setWindowTitle(title);
  }

}  // namespace fluir::editor
