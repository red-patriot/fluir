#include "editor/commands/delete_parameter.hpp"

#include <algorithm>
#include <utility>

#include "editor/diagram/parameter.hpp"

namespace fluir::editor {

  DeleteParameter::DeleteParameter(diagram::Function& function, fluir::ID id) :
    function_(function),
    id_(id),
    cut_({function.body()},
         [&parameter = **std::ranges::find(function.parameters(), id, &diagram::Parameter::id)](
           const diagram::Terminal* terminal) { return parameter.isAncestorOf(terminal); }) { }

  void DeleteParameter::redo() {
    cut_.apply();
    removed_ = function_.removeParameter(id_);
  }

  void DeleteParameter::undo() {
    function_.restoreParameter(std::move(*removed_));
    removed_.reset();
    cut_.restore();
  }

}  // namespace fluir::editor
