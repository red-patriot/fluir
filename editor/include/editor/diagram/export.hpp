#ifndef FLUIR_EDITOR_DIAGRAM_EXPORT_HPP
#define FLUIR_EDITOR_DIAGRAM_EXPORT_HPP

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor::diagram {

  /** Generates the ParseTree from `scene`. */
  pt::ParseTree exportTree(const ModuleScene& scene);

}  // namespace fluir::editor::diagram

#endif
