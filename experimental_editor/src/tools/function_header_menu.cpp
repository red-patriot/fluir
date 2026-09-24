#include "editor/tools/function_header_menu.hpp"

#include <memory>

#include "editor/core/tree_path.hpp"
#include "editor/transaction/add_parameter.hpp"
#include "editor/transaction/add_return.hpp"

namespace fluir::editor {

  std::vector<MenuItem> functionHeaderItems(const Box& hit, const EditorState& state) {
    // A function's move grip sits in its header.
    const bool header = hit.part == Part::Header || hit.part == Part::MoveGrip;
    const pt::FunctionDecl* fn = header ? functionAt(state.editor.tree(), hit.path) : nullptr;
    if (!fn) {
      return {};
    }
    return {
      MenuItem{.label = "Add parameter",
               .onClick =
                 [path = hit.path](EditorState& s) { s.editor.apply(addParameter(path, s.editor.generateID(path))); }},
      MenuItem{
        .label = "Add return",
        .onClick = [path = hit.path](EditorState& s) { s.editor.apply(addReturn(path, s.editor.generateID(path))); },
        .enabled = !(fn->output && fn->output->ret)},
    };
  }

}  // namespace fluir::editor
