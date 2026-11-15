#include "editor/commands/add_return.hpp"

#include <utility>

#include "editor/diagram/ids.hpp"

namespace fluir::editor {

  AddReturn::AddReturn(diagram::Function& function) :
    function_(function),
    hadOutput_(function.signature().output.has_value()),
    data_{.id = diagram::nextId(*function.body()), .typeName = "I32"} { }

  void AddReturn::redo() {
    if (removed_) {
      function_.restoreReturn(std::move(*removed_));
      removed_.reset();
    } else {
      function_.addReturn(data_);
    }
  }

  void AddReturn::undo() {
    removed_ = function_.removeReturn();
    if (!hadOutput_) {
      function_.dropEmptyOutput();
    }
  }

}  // namespace fluir::editor
