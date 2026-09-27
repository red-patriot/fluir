#include "editor/core/module_editor.hpp"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <utility>
#include <variant>

#include "bytecode/version.hpp"
#include "compiler/utility/context.hpp"
#include "editor/core/loader.hpp"
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

  void ModuleEditor::load(std::optional<std::filesystem::path> program, et::ParseTree tree) {
    program_ = std::move(program);
    tree_ = std::move(tree);
    undone_.clear();
    redone_.clear();
    reload();
  }

  void ModuleEditor::unload() { intelligence_.unload(program_); }

  void ModuleEditor::setProgram(std::filesystem::path program) {
    unload();
    program_ = std::move(program);
    reload();
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
    reload();
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
      reload();
      return false;
    }
    push(redone_, std::move(edit));
    reload();
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
      reload();
      return false;
    }
    push(undone_, std::move(edit));
    reload();
    return true;
  }

  void ModuleEditor::reload() { intelligence_.load(program_, tree_); }

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
    const et::Block* block = blockOf(tree_, body);
    if (block == nullptr) {
      return INVALID_ID;
    }
    const auto scan = [&top](const et::Block& scope) {
      for (const auto& [id, node] : scope.nodes) {
        top = std::max(top, id);
      }
      for (const auto& [id, conduit] : scope.conduits) {
        top = std::max(top, id);
      }
    };
    scan(*block);

    if (isBranchPath(body)) {
      // Conditional branches have separate ID spaces, but just use a single set of Ids for simplicity
      if (const auto* conditional = std::get_if<et::Conditional>(nodeAt(tree_, parentOf(body)))) {
        for (const fluir::ID branch : {THEN_BRANCH_ID, ELSE_BRANCH_ID}) {
          if (const et::Block* scope = branchBlock(*conditional, branch)) {
            scan(*scope);
          }
        }
        top = std::max(top, conditional->condition.innerId);
        for (const auto* ports : {&conditional->inputs, &conditional->outputs}) {
          for (const et::BlockPort& port : *ports) {
            top = std::max(top, port.innerId);
          }
        }
      }
    }
    if (const et::FunctionDecl* fn = functionAt(tree_, body)) {
      if (fn->input) {
        for (const et::FunctionDecl::Parameter& param : fn->input->parameters) {
          top = std::max(top, param.id);
        }
      }
      if (fn->output && fn->output->ret) {
        top = std::max(top, fn->output->ret->id);
      }
    }
    return top + 1;
  }

  std::optional<ModuleEditor> openModule(const std::filesystem::path& program) {
    fluir::Context ctx{.currentFile = program, .ignoreVersionChecks = true};
    LoadResult result = loadFile(ctx, program);
    if (!result.tree) {
      return std::nullopt;
    }
    ModuleEditor editor;
    editor.load(program, std::move(*result.tree));
    return editor;
  }

  ModuleEditor newModule() {
    et::ParseTree tree;
    tree.header.version = fluir::CURRENT_VERSION;
    ModuleEditor editor;
    editor.load(std::nullopt, std::move(tree));
    return editor;
  }

}  // namespace fluir::editor
