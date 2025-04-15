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

    friend bool operator==(const Diagnostic&, const Diagnostic&) = default;
  };

  class Diagnostics : public std::vector<Diagnostic> {
   public:
    void emitNote(std::string message) {
      this->emplace_back(Diagnostic{Diagnostic::NOTE,
                                    std::move(message)});
    }
    void emitWarning(std::string message) {
      this->emplace_back(Diagnostic{Diagnostic::WARNING,
                                    std::move(message)});
    }
    void emitError(std::string message) {
      this->emplace_back(Diagnostic{Diagnostic::ERROR,
                                    std::move(message)});
    }
    void emitInternalError(std::string message) {
      this->emplace_back(Diagnostic{Diagnostic::INTERNAL_ERROR,
                                    std::move(message)});
    }
  };
}  // namespace fluir

#endif
