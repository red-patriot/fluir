#include "editor/commands/delete_return.hpp"

#include <utility>

#include "editor/diagram/return.hpp"

namespace fluir::editor {

  DeleteReturn::DeleteReturn(diagram::Function& function) :
    function_(function), cut_({function.body()}, [&ret = *function.ret()](const diagram::Terminal* terminal) {
      return ret.isAncestorOf(terminal);
    }) { }

  void DeleteReturn::redo() {
    cut_.apply();
    removed_ = function_.removeReturn();
  }

  void DeleteReturn::undo() {
    function_.restoreReturn(std::move(*removed_));
    removed_.reset();
    cut_.restore();
  }

}  // namespace fluir::editor
