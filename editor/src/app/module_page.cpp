#include "editor/app/module_page.hpp"

#include <filesystem>
#include <fstream>
#include <QAction>
#include <QFileDialog>
#include <QKeySequence>
#include <QMessageBox>
#include <QSizePolicy>
#include <QToolBar>
#include <stdexcept>
#include <utility>

#include "editor/core/parse_tree_writer.hpp"
#include "editor/diagram/build.hpp"
#include "editor/diagram/export.hpp"
#include "ui_module_page.h"

namespace fluir::editor {
  namespace {

    const QString FILTER = QObject::tr("Fluir Program (*.fl)");

  }  // namespace

  ModulePage::ModulePage(const EditorContext& ctx,
                         std::optional<std::filesystem::path> program,
                         const pt::ParseTree& tree,
                         QWidget* parent) :
    QMainWindow(parent),
    ui_(std::make_unique<Ui::modulePage>()),
    program_(std::move(program)),
    undoStack_(new QUndoStack(this)),
    scene_(new ModuleScene(ctx, *undoStack_, intelligence_, this)),
    view_(new GraphView(ctx, this)) {
    if (!diagram::buildScene(tree, *scene_)) {
      throw std::invalid_argument("not a valid diagram");
    }
    setWindowFlags(Qt::Widget);  // QMainWindow always starts as a window.
    ui_->setupUi(this);
    // Platform-specific shortcuts are not in the .ui.
    ui_->saveAs->setShortcut(QKeySequence::SaveAs);
    ui_->undo->setShortcut(QKeySequence::Undo);
    ui_->redo->setShortcut(QKeySequence::Redo);
    // Designer cannot place a widget between actions, so the spacer is built here.
    auto* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui_->toolBar->insertWidget(ui_->exit, spacer);
    setCentralWidget(view_);

    connect(ui_->save, &QAction::triggered, this, &ModulePage::save);
    connect(ui_->saveAs, &QAction::triggered, this, &ModulePage::saveAsDialog);
    connect(ui_->undo, &QAction::triggered, undoStack_, &QUndoStack::undo);
    connect(ui_->redo, &QAction::triggered, undoStack_, &QUndoStack::redo);
    connect(undoStack_, &QUndoStack::canUndoChanged, ui_->undo, &QAction::setEnabled);
    connect(undoStack_, &QUndoStack::canRedoChanged, ui_->redo, &QAction::setEnabled);
    connect(ui_->exit, &QAction::triggered, this, &ModulePage::closeRequested);

    view_->setScene(scene_);
    view_->fit();
  }

  ModulePage::~ModulePage() = default;

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
