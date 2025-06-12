module;

#include <string>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"

export module fluir.models.asg.declaration;
export import fluir.models.asg.node;

namespace fluir::asg {
  export struct FunctionDecl {
    ID id;
    FlowGraphLocation location;
    std::string name;

    DataFlowGraph statements;
  };

  export using Declaration = FunctionDecl;  // TODO: Support other declarations
}  // namespace fluir::asg
