#include "editor/app/module_document.hpp"

#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <utility>

#include "editor/core/parse_tree_writer.hpp"

namespace fluir::editor {
  namespace {

    bool write(const et::ParseTree& tree, const std::filesystem::path& path) {
      std::ofstream ofs(path);
      if (!ofs) {
        return false;
      }
      ParseTreeWriter writer(ofs);
      writer.write(tree);
      return writer.good();
    }

  }  // namespace

  ModuleDocument::ModuleDocument(ModuleEditor editor, QObject* parent) :
    QObject(parent), editor_(std::move(editor)), canUndo_(editor_.canUndo()), canRedo_(editor_.canRedo()) { }

  ModuleDocument::~ModuleDocument() { editor_.unload(); }

  bool ModuleDocument::apply(std::unique_ptr<Transaction> edit) {
    if (!editor_.apply(std::move(edit))) {
      return false;
    }
    announce();
    return true;
  }

  void ModuleDocument::record(std::unique_ptr<Transaction> edit) {
    editor_.record(std::move(edit));
    announce();
  }

  bool ModuleDocument::undo() {
    if (!editor_.canUndo()) {
      return false;
    }
    const bool undone = editor_.undo();
    announce();
    return undone;
  }

  bool ModuleDocument::redo() {
    if (!editor_.canRedo()) {
      return false;
    }
    const bool redone = editor_.redo();
    announce();
    return redone;
  }

  void ModuleDocument::touch(const std::function<void(et::ParseTree&)>& change) {
    change(editor_.tree());
    emit changed();
  }

  bool ModuleDocument::save() {
    const auto& program = editor_.program();
    return program && write(editor_.tree(), *program);
  }

  bool ModuleDocument::saveAs(const std::filesystem::path& program) {
    if (!write(editor_.tree(), program)) {
      return false;
    }
    editor_.setProgram(program);
    emit programChanged();
    return true;
  }

  void ModuleDocument::announce() {
    emit changed();
    if (editor_.canUndo() != canUndo_) {
      canUndo_ = editor_.canUndo();
      emit canUndoChanged(canUndo_);
    }
    if (editor_.canRedo() != canRedo_) {
      canRedo_ = editor_.canRedo();
      emit canRedoChanged(canRedo_);
    }
  }

}  // namespace fluir::editor
