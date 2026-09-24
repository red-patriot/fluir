#include "editor/transaction/add_parameter.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
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
    // A deleted parameter can leave param{size + 1} taken; bump past it.
    std::size_t n = params.size() + 1;
    while (std::ranges::any_of(params, [n](const auto& param) { return param.name == "param" + std::to_string(n); })) {
      ++n;
    }
    params.push_back({.id = id_, .index = index, .name = "param" + std::to_string(n), .typeName = "I32"});
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

  std::unique_ptr<Transaction> addParameter(fluir::FullID path, fluir::ID newId) {
    return std::make_unique<AddParameter>(std::move(path), newId);
  }

}  // namespace fluir::editor
