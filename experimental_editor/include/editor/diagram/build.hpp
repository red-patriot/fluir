#ifndef FLUIR_EDITOR_DIAGRAM_BUILD_HPP
#define FLUIR_EDITOR_DIAGRAM_BUILD_HPP

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor::diagram {

  /** Replaces `scene`'s contents with `tree`'s diagram.
   * \return false, when the build fails, true when it succeeds. */
  bool buildScene(const pt::ParseTree& tree, ModuleScene& scene);

}  // namespace fluir::editor::diagram

#endif
