#ifndef FLUIR_COMPILER_UTILITY_DIAGNOSTICS_HPP
#define FLUIR_COMPILER_UTILITY_DIAGNOSTICS_HPP

#include <memory>
#include <string>
#include <vector>

namespace fluir {
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

    Level level;
    std::string message;
    std::unique_ptr<Location> where{nullptr};

    friend bool operator==(const Diagnostic& lhs, const Diagnostic& rhs) {
      return lhs.level == rhs.level && lhs.message == rhs.message;
    }
  };

  class Diagnostics : public std::vector<Diagnostic> {
   public:
    void emitNote(std::string message, std::unique_ptr<Diagnostic::Location> where = nullptr);
    void emitWarning(std::string message, std::unique_ptr<Diagnostic::Location> where = nullptr);
    void emitError(std::string message, std::unique_ptr<Diagnostic::Location> where = nullptr);
    void emitInternalError(std::string message, std::unique_ptr<Diagnostic::Location> where = nullptr);
  };
}  // namespace fluir

#endif
