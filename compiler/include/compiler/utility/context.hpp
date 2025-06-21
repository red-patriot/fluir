#ifndef FLUIR_COMPILER_UTILITY_CONTEXT_HPP
#define FLUIR_COMPILER_UTILITY_CONTEXT_HPP

#include "compiler/utility/diagnostics.hpp"
#include "compiler/utility/results.hpp"

namespace fluir {
  /** The context of compilation. */
  struct Context {
    Diagnostics diagnostics{}; /**< The diagnostics produced by the compilation process */
  };
}  // namespace fluir

#endif