#include "editor/tools/rail_menu.hpp"

#include <memory>

#include "editor/core/tree_path.hpp"
#include "editor/transaction/delete.hpp"
#include "editor/view/graph_layout.hpp"

namespace fluir::editor {

  std::vector<MenuItem> railItems(const Box& hit, Vec2 world, const EditorState& state) {
    const pt::ParseTree& tree = state.editor.tree();
    if (hit.part != Part::Body || hit.path.size() != 1 || functionAt(tree, hit.path) == nullptr) {
      return {};
    }
    // Rails are not hittable, so the press lands on the function's body; find the rail under it.
    const std::vector<Box> boxes = layoutGraph(tree, state.ctx.layout);
    const Box* rail = railAt(boxes, hit.path, world);
    if (rail == nullptr) {
      return {};
    }
    return {
      MenuItem{.label = "Delete", .onClick = [path = rail->path](EditorState& s) { s.editor.apply(deleteAt(path)); }}};
  }

}  // namespace fluir::editor
