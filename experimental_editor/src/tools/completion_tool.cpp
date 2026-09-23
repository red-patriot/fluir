#include "editor/tools/completion_tool.hpp"

#include <cmath>
#include <memory>
#include <utility>
#include <vector>

#include "editor/core/graph_geometry.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/tools/completion_modal.hpp"
#include "editor/tools/menu_popup.hpp"

namespace fluir::editor {

  bool CompletionTool::onEvent(const InputEvent& event, EditorState& state, std::span<const Box> boxes) {
    if (event.type != InputEvent::Type::MouseDown || event.button != InputEvent::Button::Right) {
      return false;
    }
    const EditorContext::Layout& layout = state.ctx.layout;
    const Vec2 world = state.view.screenToWorld(event.pos);
    const Box* hit = hitAt(boxes, world);
    FullID body;
    Vec2 origin;
    int z = 0;  // top-level parent
    if (hit != nullptr) {
      // Completions go into a container's body.
      const pt::ParseTree& tree = state.editor.tree();
      // A branch has no geometry of its own: its box is already the content area under its
      // conditional's header, and its z is the conditional's.
      const bool branch = hit->part == Part::Branch;
      const FlowGraphLocation* location = locationAt(tree, branch ? parentOf(hit->path) : hit->path);
      if ((hit->part != Part::Body && !branch) || location == nullptr || blockOf(tree, hit->path) == nullptr) {
        return false;
      }
      // A declaration's or node's hit box is its frame, so its content starts under the header.
      origin = branch ? hit->world.topLeft() : bodyOrigin(hit->world.topLeft(), layout.headerH());
      if (world.y < origin.y) {
        return false;  // header
      }
      body = hit->path;
      z = location->z;
    }
    std::vector<Completion> completions = state.intelligence.completions(state.editor.tree(), body);
    if (completions.empty()) {
      return false;
    }
    // The modal places picks one z above the parent.
    const Vec2 units = (world - origin) / layout.unitPx;
    const Coordinate where{static_cast<int>(std::lround(units.x)), static_cast<int>(std::lround(units.y)), z};
    state.popup =
      std::make_unique<CompletionModal>(std::move(completions), popupBounds(state), state.text, where, std::move(body));
    return false;  // tracked, never consumed
  }

}  // namespace fluir::editor
