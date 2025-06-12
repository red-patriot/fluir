module;

#include <vector>

export module fluir.models.asg;
export import fluir.models.asg.declaration;

namespace fluir::asg {
  export struct AbstractSyntaxGraph {
    std::vector<Declaration> declarations;
  };

  export using ASG = AbstractSyntaxGraph;
}  // namespace fluir::asg
