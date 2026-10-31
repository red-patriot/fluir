#include "editor/app/module_page.hpp"

#include <filesystem>
#include <fstream>
#include <QAction>
#include <QFileDialog>
#include <QKeySequence>
#include <QMessageBox>
#include <QSizePolicy>
#include <QToolBar>
#include <QVBoxLayout>
#include <stdexcept>
#include <utility>

#include "editor/core/parse_tree_writer.hpp"
#include "editor/diagram/build.hpp"
#include "editor/diagram/export.hpp"

namespace fluir::editor {
  namespace {

    const QString FILTER = QObject::tr("Fluir Program (*.fl)");

    QAction* toolbarAction(QToolBar* bar,
                           QAction* action,
                           const std::string_view name,
                           const QKeySequence& shortcut = {}) {
      bar->addAction(action);
      action->setObjectName(name);
      action->setShortcut(shortcut);
      return action;
    }

    QAction* toolbarAction(QToolBar* bar,
                           const QString& text,
                           const std::string_view name,
                           const QKeySequence& shortcut = {}) {
      return toolbarAction(bar, new QAction(text, bar), name, shortcut);
    }

  }  // namespace

  ModulePage::ModulePage(const EditorContext& ctx,
                         std::optional<std::filesystem::path> program,
                         const pt::ParseTree& tree,
                         QWidget* parent) :
    QWidget(parent),
    program_(std::move(program)),
    undoStack_(new QUndoStack(this)),
    scene_(new ModuleScene(ctx, *undoStack_, this)),
    view_(new GraphView(ctx, this)) {
    if (!diagram::buildScene(tree, *scene_)) {
      throw std::invalid_argument("not a valid diagram");
    }
    auto* bar = new QToolBar(tr("File"), this);
    QAction* save = toolbarAction(bar, tr("Save"), "save", QKeySequence::Save);
    QAction* saveAs = toolbarAction(bar, tr("Save As"), "saveAs", QKeySequence::SaveAs);
    toolbarAction(bar, undoStack_->createUndoAction(bar, tr("Undo")), "undo", QKeySequence::Undo);
    toolbarAction(bar, undoStack_->createRedoAction(bar, tr("Redo")), "redo", QKeySequence::Redo);
    auto* spacer = new QWidget(bar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    bar->addWidget(spacer);
    QAction* exit = toolbarAction(bar, tr("Exit"), "exit");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(bar);
    layout->addWidget(view_);

    connect(save, &QAction::triggered, this, &ModulePage::save);
    connect(saveAs, &QAction::triggered, this, &ModulePage::saveAsDialog);
    connect(exit, &QAction::triggered, this, &ModulePage::closeRequested);

    view_->setScene(scene_);
    view_->fit();
  }

  bool ModulePage::saveAs(std::filesystem::path program) {
    if (!program.has_extension()) {
      program.replace_extension("fl");
    }
    if (!write(program)) {
      return false;
    }
    program_ = std::move(program);
    emit programChanged();
    return true;
  }

  void ModulePage::save() {
    if (!program_) {
      saveAsDialog();
    } else if (!write(*program_)) {
      QMessageBox::warning(this, tr("Save"), tr("The module could not be saved."));
    }
  }

  void ModulePage::saveAsDialog() {
    const QString chosen = QFileDialog::getSaveFileName(this, tr("Save As"), {}, FILTER);
    if (!chosen.isEmpty() && !saveAs(chosen.toStdU16String())) {
      QMessageBox::warning(this, tr("Save As"), tr("The module could not be saved to %1.").arg(chosen));
    }
  }

  bool ModulePage::write(const std::filesystem::path& program) {
    std::ofstream ofs(program);
    if (!ofs) {
      return false;
    }
    ParseTreeWriter writer(ofs);
    writer.write(diagram::exportTree(*scene_));
    if (!writer.good()) {
      return false;
    }
    undoStack_->setClean();
    return true;
  }

}  // namespace fluir::editor
