#ifndef FLUIR_COMPILER_UTILITY_DIAGNOSTIC_INTERNAL_ERROR_HPP
#define FLUIR_COMPILER_UTILITY_DIAGNOSTIC_INTERNAL_ERROR_HPP

#include <stdexcept>
#include <string_view>

namespace fluir::diagnostic {
  struct InternalError final : public std::runtime_error {
    using std::runtime_error::runtime_error;
  };

  inline void emitInternalError(std::string_view message) { throw InternalError{std::string(message)}; }
}  // namespace fluir::diagnostic

#endif
