#ifndef FLUIR_COMPILER_AST_DECLARATION_HPP
#define FLUIR_COMPILER_AST_DECLARATION_HPP

#include <string>
#include <vector>

#include "compiler/frontend/ast/node.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"

namespace fluir::ast {
  struct FunctionDecl {
    ID id;
    FlowGraphLocation location;
    std::string name;

    DataFlowGraph statements;
  };

  using Declaration = FunctionDecl;  // TODO: Support other declarations
}  // namespace fluir::ast

#endif
