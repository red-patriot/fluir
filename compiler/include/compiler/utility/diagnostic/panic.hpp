#ifndef FLUIR_COMPILER_UTILITY_DIAGNOSTIC_PANIC_HPP
#define FLUIR_COMPILER_UTILITY_DIAGNOSTIC_PANIC_HPP

#include <concepts>

namespace fluir::diagnostic {
  /** A unique object to throw to initiate a panic in the compiler. */
  struct Panic { };

  /** Starts a panic*/
  [[noreturn]] inline void startPanic() { throw Panic{}; }

  namespace detail {
    struct PanicGuard {
      template <typename Func>
        requires std::invocable<Func>
      friend void operator+(PanicGuard, Func&& func) {
        try {
          func();
        } catch (const Panic&) { }
      }
    };
  }  // namespace detail

  /** Creates a new block used to synchronize a panic. */
#define FLUIR_SYNCHRONIZE_PANIC(Sink) ::fluir::diagnostic::detail::PanicGuard{} + [&]()
}  // namespace fluir::diagnostic

#endif
