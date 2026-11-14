#ifndef FLUIR_EDITOR_DIAGRAM_IDS_HPP
#define FLUIR_EDITOR_DIAGRAM_IDS_HPP

#include "compiler/models/id.hpp"
#include "editor/diagram/scope.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor::diagram {

  /** One past the largest id in `scope`'s id space. */
  fluir::ID nextId(const Scope& scope);

  /** One past the largest top-level id in `scene`. */
  fluir::ID nextId(const ModuleScene& scene);

}  // namespace fluir::editor::diagram

#endif
