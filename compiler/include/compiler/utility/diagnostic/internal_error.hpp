#ifndef FLUIR_COMPILER_UTILITY_DIAGNOSTIC_INTERNAL_ERROR_HPP
#define FLUIR_COMPILER_UTILITY_DIAGNOSTIC_INTERNAL_ERROR_HPP

#include <stdexcept>
#include <string_view>

namespace fluir::diagnostic {
  /** An exception type to represent an internal error in the compiler.
   * Each instance of this being thrown represents a bug in the compiler.
   */
  struct InternalError final : public std::runtime_error {
    using std::runtime_error::runtime_error;
  };

  /** Emits an internal error. */
  [[noreturn]] inline void emitInternalError(std::string_view message) { throw InternalError{std::string(message)}; }
}  // namespace fluir::diagnostic

#endif
