#ifndef FLUIR_COMPILER_UTILITY_PASS_HPP
#define FLUIR_COMPILER_UTILITY_PASS_HPP

#include <concepts>
#include <functional>
#include <type_traits>

#include "compiler/utility/context.hpp"

namespace fluir {
  template <typename T>
  struct DataWithContext {
    Context ctx;
    T data;
  };

  template <typename T, typename F>
  DataWithContext<std::decay_t<std::invoke_result_t<F, Context&, T>>> operator|(DataWithContext<T>&& data, F f) {
    auto result = f(data.ctx, data.data);
    return DataWithContext{std::move(data.ctx), std::move(result)};
  }

  template <typename T>
  DataWithContext<std::decay_t<T>> addContext(Context& ctx, T&& t) {
    return DataWithContext<std::decay_t<T>>{ctx, std::forward<T>(t)};
  }
}  // namespace fluir

#endif