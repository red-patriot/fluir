#include "editor/commands/delete_conduit.hpp"

namespace fluir::editor {

  DeleteConduit::DeleteConduit(diagram::Conduit& conduit) : conduit_(conduit), removed_(conduit) { }

  void DeleteConduit::redo() {
    conduit_.detach();
    removed_.remove();
  }

  void DeleteConduit::undo() {
    removed_.restore();
    conduit_.attach();
  }

}  // namespace fluir::editor
