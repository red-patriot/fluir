#pragma once

#include <optional>
#include <span>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/tree.hpp"
#include "editor/core/viewport.hpp"
#include "editor/view/graph_layout.hpp"

namespace fluir::editor {

  /**
   * Draws `boxes` in order, looking up labels in `tree`.
   * @param view Root view whose viewport maps world space to the screen
   * @param tree The model the boxes were laid out from; supplies names, literals and rail labels
   * @param boxes Output of `layoutGraph(tree)`, in paint order
   * @param selection Path of the selected function or node, outlined when drawn; nullopt for none
   * @param ctx Theme colors and layout metrics
   */
  void drawGraph(const Subview& view,
                 const et::ParseTree& tree,
                 std::span<const Box> boxes,
                 const std::optional<FullID>& selection,
                 const EditorContext& ctx);

}  // namespace fluir::editor
