#include "editor/app/main_window.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <QAction>
#include <QFileDialog>
#include <QKeySequence>
#include <QMessageBox>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QToolBar>
#include <utility>

#include "editor/app/splash_widget.hpp"
#include "editor/scene/graph_view.hpp"

namespace fluir::editor {
  namespace {

    const QString FILTER = QObject::tr("Fluir Program (*.fl)");

    QAction* toolbarAction(QToolBar* bar, const QString& text, const char* name, const QKeySequence& shortcut = {}) {
      QAction* action = bar->addAction(text);
      action->setObjectName(name);
      action->setShortcut(shortcut);
      return action;
    }

  }  // namespace

  MainWindow::MainWindow(const EditorContext& ctx, QWidget* parent) :
    QMainWindow(parent),
    pages_(new QStackedWidget(this)),
    splash_(new SplashWidget(pages_)),
    graph_(new GraphView(ctx, pages_)) {
    pages_->addWidget(splash_);
    pages_->addWidget(graph_);
    setCentralWidget(pages_);
    resize(ctx.window.width, ctx.window.height);

    toolbar_ = addToolBar(tr("File"));
    toolbar_->setMovable(false);
    QToolBar* bar = toolbar_;
    save_ = toolbarAction(bar, tr("Save"), "save", QKeySequence::Save);
    saveAs_ = toolbarAction(bar, tr("Save As"), "saveAs", QKeySequence::SaveAs);
    undo_ = toolbarAction(bar, tr("Undo"), "undo", QKeySequence::Undo);
    redo_ = toolbarAction(bar, tr("Redo"), "redo", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z));
    auto* spacer = new QWidget(bar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    bar->addWidget(spacer);
    exit_ = toolbarAction(bar, tr("Exit"), "exit");

    connect(splash_, &SplashWidget::newRequested, this, [this] { openEditor(newModule()); });
    connect(splash_, &SplashWidget::openRequested, this, &MainWindow::openDialog);
    connect(save_, &QAction::triggered, this, &MainWindow::save);
    connect(saveAs_, &QAction::triggered, this, &MainWindow::saveAsDialog);
    connect(undo_, &QAction::triggered, this, [this] { document_->undo(); });
    connect(redo_, &QAction::triggered, this, [this] { document_->redo(); });
    connect(exit_, &QAction::triggered, this, &MainWindow::closeModule);

    refresh();
  }

  MainWindow::~MainWindow() = default;

  bool MainWindow::openModule(const std::filesystem::path& program) {
    std::optional<ModuleEditor> editor = fluir::editor::openModule(program);
    if (!editor) {
      return false;
    }
    openEditor(std::move(*editor));
    return true;
  }

  bool MainWindow::saveModuleAs(std::filesystem::path program) {
    if (!program.has_extension()) {
      program.replace_extension("fl");
    }
    return document_ && document_->saveAs(program);
  }

  void MainWindow::openEditor(ModuleEditor editor) {
    document_ = std::make_unique<ModuleDocument>(std::move(editor));
    connect(document_.get(), &ModuleDocument::canUndoChanged, undo_, &QAction::setEnabled);
    connect(document_.get(), &ModuleDocument::canRedoChanged, redo_, &QAction::setEnabled);
    connect(document_.get(), &ModuleDocument::programChanged, this, &MainWindow::refresh);
    refresh();
  }

  void MainWindow::closeModule() {
    document_.reset();
    refresh();
  }

  void MainWindow::openDialog() {
    const QString chosen = QFileDialog::getOpenFileName(this, tr("Open"), {}, FILTER);
    if (!chosen.isEmpty() && !openModule(chosen.toStdU16String())) {
      QMessageBox::warning(this, tr("Open"), tr("%1 failed to load.").arg(chosen));
    }
  }

  void MainWindow::save() {
    if (!document_->editor().program()) {
      saveAsDialog();
    } else if (!document_->save()) {
      QMessageBox::warning(this, tr("Save"), tr("The module could not be saved."));
    }
  }

  void MainWindow::saveAsDialog() {
    const QString chosen = QFileDialog::getSaveFileName(this, tr("Save As"), {}, FILTER);
    if (!chosen.isEmpty() && !saveModuleAs(chosen.toStdU16String())) {
      QMessageBox::warning(this, tr("Save As"), tr("The module could not be saved to %1.").arg(chosen));
    }
  }

  void MainWindow::refresh() {
    const bool open = document_ != nullptr;
    pages_->setCurrentWidget(open ? static_cast<QWidget*>(graph_) : splash_);
    toolbar_->setVisible(open);
    for (QAction* action : {save_, saveAs_, exit_}) {
      action->setEnabled(open);
    }
    undo_->setEnabled(open && document_->editor().canUndo());
    redo_->setEnabled(open && document_->editor().canRedo());

    QString title = tr("Fluir");
    if (open) {
      const auto& program = document_->editor().program();
      title += " - " + (program ? QString::fromStdU16String(program->filename().u16string()) : tr("<new file>"));
    }
    setWindowTitle(title);
  }

}  // namespace fluir::editor
