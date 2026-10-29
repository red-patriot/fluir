#include "editor/app/main_window.hpp"

#include <filesystem>
#include <optional>
#include <QFileDialog>
#include <QMessageBox>
#include <QStackedWidget>
#include <utility>

namespace fluir::editor {

  MainWindow::MainWindow(const EditorContext& ctx, QWidget* parent) :
    QMainWindow(parent), ctx_(ctx), pages_(new QStackedWidget(this)), splash_(new SplashWidget(pages_)) {
    pages_->addWidget(splash_);
    setCentralWidget(pages_);
    resize(ctx.window.width, ctx.window.height);

    connect(splash_, &SplashWidget::newRequested, this, [this] { openPage(newModule()); });
    connect(splash_, &SplashWidget::openRequested, this, &MainWindow::openDialog);

    retitle();
  }

  bool MainWindow::openModule(const std::filesystem::path& program) {
    std::optional<ModuleEditor> editor = fluir::editor::openModule(program);
    if (!editor) {
      return false;
    }
    openPage(std::move(*editor));
    return true;
  }

  void MainWindow::openPage(ModuleEditor editor) {
    closePage();
    page_ = new ModulePage(ctx_, std::move(editor), pages_);
    pages_->addWidget(page_);
    pages_->setCurrentWidget(page_);
    connect(page_, &ModulePage::closeRequested, this, &MainWindow::closePage);
    connect(&page_->document(), &ModuleDocument::programChanged, this, &MainWindow::retitle);
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
      const auto& program = page_->document().editor().program();
      title += " - " + (program ? QString::fromStdU16String(program->filename().u16string()) : tr("<new file>"));
    }
    setWindowTitle(title);
  }

}  // namespace fluir::editor
