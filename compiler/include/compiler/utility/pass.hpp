#ifndef FLUIR_COMPILER_UTILITY_PASS_HPP
#define FLUIR_COMPILER_UTILITY_PASS_HPP

#include <concepts>
#include <functional>
#include <type_traits>

#include "compiler/utility/context.hpp"

namespace fluir {
  template <typename T, typename U>
  concept CompilerPass = requires(Context& ctx, U u) {
    { T::run(ctx, u) };
  };
}  // namespace fluir

#endif