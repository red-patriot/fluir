#include "editor/commands/delete_port.hpp"

#include <algorithm>
#include <utility>

#include "editor/diagram/port.hpp"

namespace fluir::editor {
  namespace {

    const diagram::Port& portOf(const diagram::Conditional& conditional, draw::PortRef ref) {
      return **std::ranges::find(conditional.wallPorts(), ref, &diagram::Port::ref);
    }

  }  // namespace

  DeletePort::DeletePort(diagram::Conditional& conditional, draw::PortRef ref) :
    conditional_(conditional),
    ref_(ref),
    cut_(
      {conditional.parentItem(), conditional.thenBranch(), conditional.elseBranch()},
      [&port = portOf(conditional, ref)](const diagram::Terminal* terminal) { return port.isAncestorOf(terminal); }) { }

  void DeletePort::redo() {
    cut_.apply();
    removed_ = conditional_.removePort(ref_);
  }

  void DeletePort::undo() {
    conditional_.restorePort(std::move(*removed_));
    removed_.reset();
    cut_.restore();
  }

}  // namespace fluir::editor
