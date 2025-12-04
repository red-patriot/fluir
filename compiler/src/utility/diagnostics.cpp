#include "compiler/utility/diagnostics.hpp"

#include <algorithm>

namespace fluir {
  bool isError(const Diagnostic& diagnostic) { return diagnostic.level >= Diagnostic::Level::ERROR; }

  std::string toString(const Diagnostic& diagnostic) {
    auto& [level, message, where] = diagnostic;

    if (diagnostic.where) {
      return fmt::format("[{}] {}: {}", level, where->str(), message);
    } else {
      return fmt::format("[{}]: {}", level, message);
    }
  }

  SourceLocation::SourceLocation(std::source_location source) : source_(source) { }

  std::string SourceLocation::str() const {
    return fmt::format("on line {} of '{}' in '{}'", source_.line(), source_.file_name(), source_.function_name());
  }

  void Diagnostics::emitNote(std::string message, std::shared_ptr<Diagnostic::Location> where) {
    this->emplace_back(Diagnostic{Diagnostic::Level::NOTE, std::move(message), std::move(where)});
  }
  void Diagnostics::emitWarning(std::string message, std::shared_ptr<Diagnostic::Location> where) {
    this->emplace_back(Diagnostic{Diagnostic::Level::WARNING, std::move(message), std::move(where)});
  }
  void Diagnostics::emitError(std::string message, std::shared_ptr<Diagnostic::Location> where) {
    this->emplace_back(Diagnostic{Diagnostic::Level::ERROR, std::move(message), std::move(where)});
  }
  void Diagnostics::emitInternalError(std::string message, std::shared_ptr<Diagnostic::Location> where) {
    this->emplace_back(Diagnostic{Diagnostic::Level::INTERNAL_ERROR, std::move(message), std::move(where)});
  }

  bool Diagnostics::containsErrors() const { return std::ranges::any_of(*this, isError); }
}  // namespace fluir

auto fmt::formatter<fluir::Diagnostic::Level>::format(fluir::Diagnostic::Level l,
                                                      format_context& ctx) const -> fmt::format_context::iterator {
  using enum fluir::Diagnostic::Level;

  fmt::string_view name = "<UNKNOWN>";
  switch (l) {
    case NOTE:
      name = "NOTE";
      break;
    case WARNING:
      name = "WARNING";
      break;
    case ERROR:
      name = "ERROR";
      break;
    case INTERNAL_ERROR:
      name = "INTERNAL ERROR";
      break;
  }

  return formatter<fmt::string_view>::format(name, ctx);
}
