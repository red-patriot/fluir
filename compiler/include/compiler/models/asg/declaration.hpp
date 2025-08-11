#ifndef FLUIR_COMPILER_MODELS_ASG_DECLARATION_HPP
#define FLUIR_COMPILER_MODELS_ASG_DECLARATION_HPP

#include <string>
#include <vector>

#include "compiler/models/asg/node.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"

namespace fluir::asg {
  struct FunctionDecl {
    ID id;
    FlowGraphLocation location;
    std::string name;

    DataFlowGraph statements;
  };

  using Declaration = FunctionDecl;  // TODO: Support other declarations
}  // namespace fluir::asg

#endif
