#ifndef FLUIR_COMPILER_FRONTEND_TYPE_CHECKER_HPP
#define FLUIR_COMPILER_FRONTEND_TYPE_CHECKER_HPP

#include "compiler/models/asg.hpp"
#include "compiler/utility/context.hpp"

namespace fluir {
  Results<asg::Declaration> checkTypes(Context& ctx, asg::Declaration decl);
}

#endif
