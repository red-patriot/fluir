#include "editor/transaction/add_parameter.hpp"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  bool AddParameter::execute(pt::ParseTree& tree) {
    pt::FunctionDecl* fn = functionAt(tree, path_);
    if (fn == nullptr || id_ == INVALID_ID || railTypeAt(*fn, id_) != nullptr) {
      return false;
    }
    createdInput_ = !fn->input.has_value();
    if (createdInput_) {
      fn->input.emplace();
    }
    std::vector<pt::FunctionDecl::Parameter>& params = fn->input->parameters;
    int index = 0;
    for (const pt::FunctionDecl::Parameter& param : params) {
      index = std::max(index, param.index + 1);
    }
    params.push_back(
      {.id = id_, .index = index, .name = "param" + std::to_string(params.size() + 1), .typeName = "I32"});
    return true;
  }

  bool AddParameter::unexecute(pt::ParseTree& tree) {
    pt::FunctionDecl* fn = functionAt(tree, path_);
    if (fn == nullptr || !fn->input) {
      return false;
    }
    if (std::erase_if(fn->input->parameters, [this](const auto& param) { return param.id == id_; }) == 0) {
      return false;
    }
    if (createdInput_ && fn->input->parameters.empty()) {
      fn->input = std::nullopt;
    }
    return true;
  }

}  // namespace fluir::editor
