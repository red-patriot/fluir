#include "editor/transaction/update_func_param.hpp"

#include <algorithm>
#include <utility>

#include "editor/core/identifier.hpp"
#include "editor/core/tree_path.hpp"

namespace fluir::editor {

  std::unique_ptr<UpdateFuncParamTransaction> UpdateFuncParamTransaction::rename(fluir::FullID path,
                                                                                 int index,
                                                                                 std::string name) {
    return std::unique_ptr<UpdateFuncParamTransaction>(
      new UpdateFuncParamTransaction(std::move(path), Rename{index, std::move(name)}));
  }

  std::unique_ptr<UpdateFuncParamTransaction> UpdateFuncParamTransaction::setType(fluir::FullID path,
                                                                                  fluir::ID railId,
                                                                                  std::string typeName) {
    return std::unique_ptr<UpdateFuncParamTransaction>(
      new UpdateFuncParamTransaction(std::move(path), SetType{railId, std::move(typeName)}));
  }

  bool UpdateFuncParamTransaction::execute(pt::ParseTree& tree) {
    pt::FunctionDecl* fn = functionAt(tree, path_);
    if (fn == nullptr) {
      return false;
    }
    if (auto* setType = std::get_if<SetType>(&edit_)) {
      std::string* type = railTypeAt(*fn, setType->railId);
      if (type == nullptr || setType->typeName.empty() || *type == setType->typeName) {
        return false;
      }
      std::swap(*type, setType->typeName);
      return true;
    }
    auto& rename = std::get<Rename>(edit_);
    if (!fn->input || !isValidIdentifier(rename.name)) {
      return false;
    }
    auto& params = fn->input->parameters;
    const auto param = std::ranges::find(params, rename.index, &pt::FunctionDecl::Parameter::index);
    if (param == params.end() || param->name == rename.name) {
      return false;
    }
    std::swap(param->name, rename.name);
    return true;
  }

}  // namespace fluir::editor
