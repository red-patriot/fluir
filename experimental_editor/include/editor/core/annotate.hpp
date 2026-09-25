#ifndef FLUIR_EDITOR_CORE_ANNOTATE_HPP
#define FLUIR_EDITOR_CORE_ANNOTATE_HPP

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/tree.hpp"

namespace fluir::editor {

  /** A copy of `tree` with default annotations. The only place the two instantiations meet. */
  et::ParseTree annotate(const pt::ParseTree& tree);

}  // namespace fluir::editor

#endif
