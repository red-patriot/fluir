#ifndef FLUIR_COMPILER_UTILITY_SCOPE_GUARD_HPP
#define FLUIR_COMPILER_UTILITY_SCOPE_GUARD_HPP

#include <exception>
#include <utility>

#include "compiler/utility/macros.hpp"

namespace fluir::scope_guard::detail {
  template <typename Func, bool OnSuccess, bool OnException>
  struct ScopeGuardImpl {
    explicit ScopeGuardImpl(Func&& func) :
      func_(std::forward<Func>(func)), exceptionsOnEnter_(std::uncaught_exceptions()) { }

    ~ScopeGuardImpl() noexcept(OnException) {
      if constexpr (OnSuccess) {
        if (exceptionsOnEnter_ == std::uncaught_exceptions()) {
          func_();
        }
      }
      if constexpr (OnException) {
        if (exceptionsOnEnter_ < std::uncaught_exceptions()) {
          func_();
        }
      }
    }

    Func func_;
    int exceptionsOnEnter_;
  };

  template <bool OnSuccess, bool OnException>
  struct ScopeGuardBuilder {
    template <typename Func>
    friend ScopeGuardImpl<Func, OnSuccess, OnException> operator+(const ScopeGuardBuilder&, Func&& func) {
      return ScopeGuardImpl<Func, OnSuccess, OnException>{std::forward<Func>(func)};
    }
  };

  using ScopeExit = ScopeGuardBuilder<true, true>;
  using ScopeSuccess = ScopeGuardBuilder<true, false>;
  using ScopeFailure = ScopeGuardBuilder<false, true>;
}  // namespace fluir::scope_guard::detail

#define FLUIR_SCOPE_EXIT \
  auto FLUIR_ANONYMOUS_VARIABLE(scopeExitGuard) = ::fluir::scope_guard::detail::ScopeExit{} + [&]() noexcept
#define FLUIR_SCOPE_SUCCESS \
  auto FLUIR_ANONYMOUS_VARIABLE(scopeSuccess) = ::fluir::scope_guard::detail::ScopeSuccess{} + [&]()
#define FLUIR_SCOPE_FAILURE \
  auto FLUIR_ANONYMOUS_VARIABLE(scopeFailure) = ::fluir::scope_guard::detail::ScopeFailure{} + [&]() noexcept

#endif  // FLUIR_COMPILER_SCOPE_GUARD_HPP
