#include "editor/tools/port_menu.hpp"

#include "editor/transaction/delete_port.hpp"

namespace fluir::editor {

  std::vector<MenuItem> portItems(const Box& hit, const EditorState&) {
    if (hit.part != Part::Port || !hit.port || (!hit.port->output && hit.port->index == 0)) {
      return {};
    }
    return {MenuItem{.label = "Delete port", .onClick = [path = hit.path, ref = *hit.port](EditorState& s) {
                       s.editor.apply(deletePort(path, ref));
                     }}};
  }

}  // namespace fluir::editor
