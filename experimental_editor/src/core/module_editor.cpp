#include "editor/core/module_editor.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <utility>

#include "editor/core/tree_path.hpp"

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

  fluir::ID ModuleEditor::generateID(const fluir::FullID& body) const {
    // Max ID in scope + 1, over the same scope the parser checks for duplicates.
    // TODO: This can be made more efficient
    fluir::ID top = 0;
    if (body.empty()) {
      for (const auto& [id, decl] : tree_.declarations) {
        top = std::max(top, id);
      }
      return top + 1;
    }
    const pt::Block* block = blockOf(tree_, body);
    if (block == nullptr) {
      return INVALID_ID;
    }
    for (const auto& [id, node] : block->nodes) {
      top = std::max(top, id);
    }
    for (const auto& [id, conduit] : block->conduits) {
      top = std::max(top, id);
    }
    if (const pt::FunctionDecl* fn = functionAt(tree_, body)) {
      if (fn->input) {
        for (const pt::FunctionDecl::Parameter& param : fn->input->parameters) {
          top = std::max(top, param.id);
        }
      }
      if (fn->output && fn->output->ret) {
        top = std::max(top, fn->output->ret->id);
      }
    }
    return top + 1;
  }

}  // namespace fluir::editor
