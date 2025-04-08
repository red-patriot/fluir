#ifndef FLUIR_COMPILER_DIAGNOSTIC_HPP
#define FLUIR_COMPILER_DIAGNOSTIC_HPP

#include <memory>
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

    class Where {
     public:
      virtual ~Where() = default;
      virtual std::string string() const = 0;
    };

    using enum Level;

    Level level;
    std::string message;
    std::unique_ptr<Where> where{nullptr};

    friend bool operator==(const Diagnostic&, const Diagnostic&) = default;
    friend std::ostream& operator<<(std::ostream& os, const Diagnostic& diagnostic);
  };

  using DiagnosticCollection = std::vector<Diagnostic>;

  template <typename... Args>
  inline void emitError(DiagnosticCollection& destination,
                        std::unique_ptr<Diagnostic::Where> where,
                        std::string_view message, Args&&... args) {
    destination.push_back(Diagnostic{.level = Diagnostic::ERROR,
                                     .message = fmt::vformat(message, fmt::make_format_args(args...)),
                                     .where = std::move(where)});
  }

  template <typename... Args>
  inline void emitError(DiagnosticCollection& destination,
                        std::string_view message, Args&&... args) {
    destination.push_back(Diagnostic{.level = Diagnostic::ERROR,
                                     .message = fmt::vformat(message, fmt::make_format_args(args...))});
  }
}  // namespace fluir::compiler

template <>
struct fmt::formatter<fluir::compiler::Diagnostic::Level> : formatter<fmt::string_view> {
  // parse is inherited from formatter<string_view>.

  auto format(fluir::compiler::Diagnostic::Level l, format_context& ctx) const
      -> fmt::format_context::iterator;
};

#endif
