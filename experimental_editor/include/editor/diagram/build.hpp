#ifndef FLUIR_EDITOR_DIAGRAM_BUILD_HPP
#define FLUIR_EDITOR_DIAGRAM_BUILD_HPP

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/diagram/node.hpp"
#include "editor/diagram/scope.hpp"
#include "editor/scene/module_scene.hpp"

namespace fluir::editor::diagram {

  /** Replaces `scene`'s contents with `tree`'s diagram.
   * \return false, when the build fails, true when it succeeds. */
  bool buildScene(const pt::ParseTree& tree, ModuleScene& scene);

  /** Builds `node` and its contents under `scope`, which must be in a scene. Throws if the contents do not wire up. */
  FluirNode& buildNode(const pt::Node& node, Scope& scope);

  /** Builds `decl` and its contents at `scene`'s top level. Throws if the contents do not wire up. */
  FluirNode& buildDeclaration(const pt::Declaration& decl, ModuleScene& scene);

}  // namespace fluir::editor::diagram

#endif
