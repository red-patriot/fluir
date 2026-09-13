#include "editor/core/module_editor.hpp"

#include <cstddef>
#include <memory>
#include <utility>

namespace fluir::editor {
  namespace {

    constexpr std::size_t MAX_HISTORY = 100;

    void push(std::deque<std::unique_ptr<Transaction>>& stack, std::unique_ptr<Transaction> edit) {
      stack.push_back(std::move(edit));
      if (stack.size() > MAX_HISTORY) {
        stack.pop_front();
      }
    }

  }  // namespace

  void ModuleEditor::load(pt::ParseTree tree) {
    tree_ = std::move(tree);
    undone_.clear();
    redone_.clear();
  }

  bool ModuleEditor::apply(std::unique_ptr<Transaction> edit) {
    if (!edit->execute(tree_)) {
      return false;
    }
    record(std::move(edit));
    return true;
  }

  void ModuleEditor::record(std::unique_ptr<Transaction> edit) {
    push(undone_, std::move(edit));
    redone_.clear();
  }

  bool ModuleEditor::undo() {
    if (undone_.empty()) {
      return false;
    }
    std::unique_ptr<Transaction> edit = std::move(undone_.back());
    undone_.pop_back();
    // A failed reversal means the tree is not what the history assumes.
    if (!edit->unexecute(tree_)) {
      undone_.clear();
      redone_.clear();
      return false;
    }
    push(redone_, std::move(edit));
    return true;
  }

  bool ModuleEditor::redo() {
    if (redone_.empty()) {
      return false;
    }
    std::unique_ptr<Transaction> edit = std::move(redone_.back());
    redone_.pop_back();
    if (!edit->execute(tree_)) {
      undone_.clear();
      redone_.clear();
      return false;
    }
    push(undone_, std::move(edit));
    return true;
  }

}  // namespace fluir::editor
