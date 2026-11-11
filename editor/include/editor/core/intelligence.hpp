#ifndef FLUIR_EDITOR_CORE_INTELLIGENCE_HPP
#define FLUIR_EDITOR_CORE_INTELLIGENCE_HPP

#include <string>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"

namespace fluir::editor {
  /** A completion option on the flow diagram. */
  template <typename T>
  struct Completion {
    std::string label;
    T prototype;
  };

  /** Provides completion and type intelligence over a snapshot of a program's tree. */
  class Intelligence {
   public:
    /** What may be added at the top level of `tree`. */
    std::vector<Completion<pt::Declaration>> topLevelCompletions(const pt::ParseTree& tree) const;

    /** What may be added in `scope` in `tree`. */
    std::vector<Completion<pt::Node>> completionsAt(const pt::ParseTree& tree, const fluir::FullID& scope) const;
  };

}  // namespace fluir::editor

#endif
