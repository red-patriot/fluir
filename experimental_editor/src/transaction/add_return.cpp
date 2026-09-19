#include "editor/transaction/add_return.hpp"

#include <optional>

#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  bool AddReturn::execute(pt::ParseTree& tree) {
    pt::FunctionDecl* fn = functionAt(tree, path_);
    if (fn == nullptr || id_ == INVALID_ID || (fn->output && fn->output->ret) || railTypeAt(*fn, id_) != nullptr) {
      return false;
    }
    createdOutput_ = !fn->output.has_value();
    if (createdOutput_) {
      fn->output.emplace();
    }
    fn->output->ret = pt::FunctionDecl::Return{.id = id_, .typeName = "I32"};
    return true;
  }

  bool AddReturn::unexecute(pt::ParseTree& tree) {
    pt::FunctionDecl* fn = functionAt(tree, path_);
    if (fn == nullptr || !fn->output || !fn->output->ret || fn->output->ret->id != id_) {
      return false;
    }
    if (createdOutput_) {
      fn->output = std::nullopt;
    } else {
      fn->output->ret = std::nullopt;
    }
    return true;
  }

}  // namespace fluir::editor
