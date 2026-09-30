#include "editor/tools/conditional_header_menu.hpp"

#include <algorithm>
#include <variant>

#include "editor/core/geometry.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/add_port.hpp"
#include "editor/view/draw/conditional.hpp"

namespace fluir::editor {

  namespace {

    MenuItem addPortItem(const char* label, const FullID& path, bool output) {
      return MenuItem{.label = label, .onClick = [path, output](EditorState& s) {
                        const auto* conditional = std::get_if<et::Conditional>(nodeAt(s.editor.tree(), path));
                        if (conditional == nullptr) {
                          return;
                        }
                        FullID branch = path;
                        branch.push_back(THEN_BRANCH_ID);
                        const Limits<int> limits = draw::portYLimits(*conditional, s.ctx.layout);
                        const int y = std::clamp(conditional->location.height / 2, limits.lower, limits.upper);
                        s.editor.apply(addPort(path, output, s.editor.generateID(branch), y));
                      }};
    }

  }  // namespace

  std::vector<MenuItem> conditionalHeaderItems(const Box& hit, const EditorState& state) {
    if (hit.part != Part::Header || !std::get_if<et::Conditional>(nodeAt(state.editor.tree(), hit.path))) {
      return {};
    }
    return {addPortItem("Add input", hit.path, false), addPortItem("Add output", hit.path, true)};
  }

}  // namespace fluir::editor
