#include "editor/tools/function_header_menu.hpp"

#include <memory>

#include "editor/core/graph_geometry.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/add_parameter.hpp"
#include "editor/transaction/add_return.hpp"

namespace fluir::editor {

  std::vector<MenuItem> functionHeaderItems(const Box& hit, Vec2 world, const EditorState& state) {
    const pt::FunctionDecl* fn = hit.path.size() == 1 ? functionAt(state.editor.tree(), hit.path) : nullptr;
    const EditorContext::Layout& layout = state.ctx.layout;
    if (fn == nullptr || world.y >= localRect(fn->location, layout.unitPx).y + layout.headerH()) {
      return {};
    }
    return {
      MenuItem{.label = "Add parameter",
               .onClick =
                 [path = hit.path](EditorState& s) {
                   s.editor.apply(std::make_unique<AddParameter>(path, s.editor.generateID(path)));
                 }},
      MenuItem{
        .label = "Add return",
        .onClick = [path = hit.path](
                     EditorState& s) { s.editor.apply(std::make_unique<AddReturn>(path, s.editor.generateID(path))); },
        .enabled = !(fn->output && fn->output->ret)},
    };
  }

}  // namespace fluir::editor
