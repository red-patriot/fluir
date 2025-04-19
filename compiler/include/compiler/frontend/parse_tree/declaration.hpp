#ifndef FLUIR_COMPILER_FRONTEND_PARSE_TREE_DECLARATION_HPP
#define FLUIR_COMPILER_FRONTEND_PARSE_TREE_DECLARATION_HPP

#include <string>
#include <variant>

#include "block.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"

namespace fluir::pt {
  struct FunctionDecl {
    ID id;
    FlowGraphLocation location;

    std::string name;
    Block body;
  };

  using Declaration = FunctionDecl;  // TODO: Support other top-level declarations here
}  // namespace fluir::pt

#endif
