#ifndef FLUIR_COMPILER_UTILITY_DIAGNOSTICS_HPP
#define FLUIR_COMPILER_UTILITY_DIAGNOSTICS_HPP

#include <memory>
#include <string>
#include <vector>

#include <fmt/format.h>

namespace fluir {
  /** A diagnostic emitted by the compiler */
  struct Diagnostic {
    enum class Level {
      NOTE = 0,
      WARNING = 1,
      ERROR = 2,
      INTERNAL_ERROR = 3
    };
    using enum Level;

    class Location {
     public:
      virtual ~Location() = default;
      virtual std::string str() const = 0;
    };

    Level level;                              /**< The level of diagnostic */
    std::string message;                      /**< The user-facing message to help fix the issue */
    std::shared_ptr<Location> where{nullptr}; /**< Where the issue originated from */

    friend bool operator==(const Diagnostic& lhs, const Diagnostic& rhs) {
      return lhs.level == rhs.level && lhs.message == rhs.message;
    }
  };

  bool isError(const Diagnostic& diagnostic);

  std::string toString(const Diagnostic& diagnostic);

  class Diagnostics : public std::vector<Diagnostic> {
   public:
    using vector::vector;

    void emitNote(std::string message, std::shared_ptr<Diagnostic::Location> where = nullptr);
    void emitWarning(std::string message, std::shared_ptr<Diagnostic::Location> where = nullptr);
    void emitError(std::string message, std::shared_ptr<Diagnostic::Location> where = nullptr);
    void emitInternalError(std::string message, std::shared_ptr<Diagnostic::Location> where = nullptr);

    bool containsErrors() const;
  };
}  // namespace fluir

template <>
struct fmt::formatter<fluir::Diagnostic::Level> : formatter<fmt::string_view> {
  // parse is inherited from formatter<string_view>.

  auto format(fluir::Diagnostic::Level l, format_context& ctx) const
      -> fmt::format_context::iterator;
};

#endif
