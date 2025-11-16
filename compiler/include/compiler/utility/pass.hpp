#ifndef FLUIR_COMPILER_UTILITY_PASS_HPP
#define FLUIR_COMPILER_UTILITY_PASS_HPP

#include <concepts>
#include <functional>
#include <optional>
#include <type_traits>

#include "compiler/utility/context.hpp"

namespace fluir {
  template <typename T>
  struct isOptional : std::false_type { };

  template <typename T>
  struct isOptional<std::optional<T>> : std::true_type { };

  template <typename T>
  concept OptionalSpecialization = isOptional<T>::value;

  template <typename F, typename T>
  concept CompilerPass = std::movable<T> && requires(F f, T t, Context& ctx) {
    { f(ctx, t) } -> OptionalSpecialization;
  };

  template <typename T>
    requires std::movable<T> && OptionalSpecialization<T>
  struct DataWithContext {
    Context ctx;
    T data;
  };

  template <typename T, typename F>
  auto operator|(DataWithContext<std::optional<T>> data, F f)
    requires CompilerPass<F, T>
  {
    using ReturnType = DataWithContext<std::decay_t<std::invoke_result_t<F, Context&, T>>>;

    if (data.ctx.diagnostics.containsErrors()) {
      return ReturnType{std::move(data.ctx), std::nullopt};
    }

    auto result = f(data.ctx, std::move(data.data.value()));
    return ReturnType{std::move(data.ctx), std::move(result)};
  }

  template <typename T>
  DataWithContext<std::optional<std::decay_t<T>>> addContext(Context ctx, T&& t) {
    return DataWithContext<std::optional<std::decay_t<T>>>{std::move(ctx), std::forward<T>(t)};
  }
}  // namespace fluir

#endif
