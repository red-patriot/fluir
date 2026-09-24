#include "editor/tools/rail_menu.hpp"

#include <memory>

#include "editor/transaction/delete.hpp"

namespace fluir::editor {

  std::vector<MenuItem> railItems(const Box& hit, const EditorState&) {
    if (hit.part != Part::Rail) {
      return {};
    }
    return {
      MenuItem{.label = "Delete", .onClick = [path = hit.path](EditorState& s) { s.editor.apply(deleteAt(path)); }}};
  }

}  // namespace fluir::editor
