#include "editor/commands/add_parameter.hpp"

#include <string>
#include <utility>

#include <fmt/format.h>

#include "editor/diagram/ids.hpp"

namespace fluir::editor {

  AddParameter::AddParameter(diagram::Function& function) :
    function_(function), hadInput_(function.signature().input.has_value()) {
    const int position = hadInput_ ? static_cast<int>(function.signature().input->parameters.size()) : 0;
    data_ = {.id = diagram::nextId(*function.body()),
             .index = position,
             .name = fmt::format("param{}", position + 1),
             .typeName = "I32"};
  }

  void AddParameter::redo() {
    if (removed_) {
      function_.restoreParameter(std::move(*removed_));
      removed_.reset();
    } else {
      function_.addParameter(data_);
    }
  }

  void AddParameter::undo() {
    removed_ = function_.removeParameter(data_.id);
    if (!hadInput_) {
      function_.dropEmptyInput();
    }
  }

}  // namespace fluir::editor
