#ifndef FLUIR_COMPILER_MODELS_ASG_HPP
#define FLUIR_COMPILER_MODELS_ASG_HPP

#include <vector>

#include "compiler/models/asg/declaration.hpp"

namespace fluir::asg {
  struct AbstractSyntaxGraph {
    std::vector<Declaration> declarations{};
  };

  using ASG = AbstractSyntaxGraph;
}  // namespace fluir::asg

#endif
