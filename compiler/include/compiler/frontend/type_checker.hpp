#ifndef FLUIR_COMPILER_FRONTEND_TYPE_CHECKER_HPP
#define FLUIR_COMPILER_FRONTEND_TYPE_CHECKER_HPP

#include "compiler/models/asg.hpp"
#include "compiler/utility/context.hpp"

namespace fluir {
  Results<asg::ASG> typeCheck(Context& ctx, asg::ASG graph);
  Results<asg::Declaration> checkDeclType(Context& ctx, asg::Declaration decl);
}  // namespace fluir

#endif
