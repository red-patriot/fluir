#ifndef FLUIR_COMPILER_DIAGNOSTIC_HPP
#define FLUIR_COMPILER_DIAGNOSTIC_HPP

#include <string>
#include <vector>

#include <fmt/format.h>

namespace fluir::compiler {
  struct Diagnostic {
    enum class Level {
      NOTE,
      WARNING,
      ERROR,
      INTERNAL_ERROR,
    };
    using enum Level;

    Level level;
    std::string message;

    friend bool operator==(const Diagnostic&, const Diagnostic&) = default;
  };

  using DiagnosticCollection = std::vector<Diagnostic>;

  template <typename... Args>
  inline void emitError(DiagnosticCollection& destination,
                        std::string_view message, Args&&... args) {
    destination.push_back(Diagnostic{.level = Diagnostic::ERROR,
                                     .message = fmt::vformat(message, fmt::make_format_args(args...))});
  }
}  // namespace fluir::compiler

#endif
