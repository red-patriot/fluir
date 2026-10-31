#include "editor/commands/add_port.hpp"

#include <utility>

#include "editor/diagram/grid.hpp"
#include "editor/diagram/ids.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor {

  AddPort::AddPort(diagram::Conditional& conditional, bool output) :
    conditional_(conditional), output_(output), data_{} {
    const int height = conditional.location().height;
    data_ = {.innerId = diagram::nextId(*conditional.thenBranch()),
             .y = diagram::clamp(height / 2, draw::portYLimits(height, ModuleScene::of(conditional).context().layout))};
  }

  void AddPort::redo() {
    if (removed_) {
      conditional_.restorePort(std::move(*removed_));
      removed_.reset();
    } else {
      ref_ = conditional_.addPort(output_, data_);
    }
  }

  void AddPort::undo() { removed_ = conditional_.removePort(ref_); }

}  // namespace fluir::editor
