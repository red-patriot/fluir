#ifndef FLUIR_COMPILER_UTILITY_DIAGNOSTIC_PANIC_HPP
#define FLUIR_COMPILER_UTILITY_DIAGNOSTIC_PANIC_HPP

#include <concepts>

namespace fluir::diagnostic {
  struct PanicMode { };

  inline void startPanic() { throw PanicMode{}; }

  namespace detail {
    struct PanicGuard {
      template <typename Func>
        requires std::invocable<Func>
      friend void operator+(PanicGuard, Func&& func) {
        try {
          func();
        } catch (const PanicMode&) { }
      }
    };
  }  // namespace detail

#define FLUIR_SYNCHRONIZE_PANIC(Sink) ::fluir::diagnostic::detail::PanicGuard{} + [&]()
}  // namespace fluir::diagnostic

#endif
