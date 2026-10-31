#ifndef FLUIR_EDITOR_DIAGRAM_IDS_HPP
#define FLUIR_EDITOR_DIAGRAM_IDS_HPP

#include "compiler/models/id.hpp"
#include "editor/diagram/scope.hpp"

namespace fluir::editor::diagram {

  /** One past the largest id in `scope`'s id space. */
  fluir::ID nextId(const Scope& scope);

}  // namespace fluir::editor::diagram

#endif
