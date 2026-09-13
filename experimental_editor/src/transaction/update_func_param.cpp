#include "editor/transaction/update_func_param.hpp"

#include <algorithm>
#include <utility>

#include "editor/core/identifier.hpp"
#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  bool UpdateFuncParamTransaction::execute(pt::ParseTree& tree) {
    pt::FunctionDecl* fn = functionAt(tree, path_);
    if (fn == nullptr || !fn->input || !isValidIdentifier(name_)) {
      return false;
    }
    auto& params = fn->input->parameters;
    const auto param = std::ranges::find(params, index_, &pt::FunctionDecl::Parameter::index);
    if (param == params.end() || param->name == name_) {
      return false;
    }
    std::swap(param->name, name_);
    return true;
  }

}  // namespace fluir::editor
