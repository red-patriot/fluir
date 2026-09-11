#pragma once

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/actors/scene.hpp"

namespace fluir::editor {

  /** The parse tree the scene currently represents, ready for the writer. */
  pt::ParseTree sceneToParseTree(const GraphScene& scene, const pt::Header& header);

}  // namespace fluir::editor
