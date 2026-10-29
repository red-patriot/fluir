#include "editor/app/module_page.hpp"

#include <filesystem>
#include <QAction>
#include <QFileDialog>
#include <QKeySequence>
#include <QMessageBox>
#include <QSizePolicy>
#include <QToolBar>
#include <QVBoxLayout>
#include <utility>

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

  ModulePage::ModulePage(const EditorContext& ctx, ModuleEditor editor, QWidget* parent) :
    QWidget(parent), document_(std::move(editor)), scene_(new GraphScene(ctx, this)), view_(new GraphView(ctx, this)) {
    auto* bar = new QToolBar(tr("File"), this);
    QAction* save = toolbarAction(bar, tr("Save"), "save", QKeySequence::Save);
    QAction* saveAs = toolbarAction(bar, tr("Save As"), "saveAs", QKeySequence::SaveAs);
    QAction* undo = toolbarAction(bar, tr("Undo"), "undo", QKeySequence::Undo);
    QAction* redo = toolbarAction(bar, tr("Redo"), "redo", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z));
    auto* spacer = new QWidget(bar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    bar->addWidget(spacer);
    QAction* exit = toolbarAction(bar, tr("Exit"), "exit");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(bar);
    layout->addWidget(view_);

    undo->setEnabled(document_.editor().canUndo());
    redo->setEnabled(document_.editor().canRedo());
    connect(save, &QAction::triggered, this, &ModulePage::save);
    connect(saveAs, &QAction::triggered, this, &ModulePage::saveAsDialog);
    connect(undo, &QAction::triggered, &document_, &ModuleDocument::undo);
    connect(redo, &QAction::triggered, &document_, &ModuleDocument::redo);
    connect(exit, &QAction::triggered, this, &ModulePage::closeRequested);
    connect(&document_, &ModuleDocument::canUndoChanged, undo, &QAction::setEnabled);
    connect(&document_, &ModuleDocument::canRedoChanged, redo, &QAction::setEnabled);
    connect(&document_, &ModuleDocument::changed, scene_, [this] { scene_->reconcile(document_.editor().tree()); });

    scene_->reconcile(document_.editor().tree());
    view_->setScene(scene_);
    view_->fit();
  }

  bool ModulePage::saveAs(std::filesystem::path program) {
    if (!program.has_extension()) {
      program.replace_extension("fl");
    }
    return document_.saveAs(program);
  }

  void ModulePage::save() {
    if (!document_.editor().program()) {
      saveAsDialog();
    } else if (!document_.save()) {
      QMessageBox::warning(this, tr("Save"), tr("The module could not be saved."));
    }
  }

  void ModulePage::saveAsDialog() {
    const QString chosen = QFileDialog::getSaveFileName(this, tr("Save As"), {}, FILTER);
    if (!chosen.isEmpty() && !saveAs(chosen.toStdU16String())) {
      QMessageBox::warning(this, tr("Save As"), tr("The module could not be saved to %1.").arg(chosen));
    }
  }

}  // namespace fluir::editor
