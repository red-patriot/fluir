#include "compiler/utility/diagnostics.hpp"

namespace fluir {
  void Diagnostics::emitNote(std::string message, std::unique_ptr<Diagnostic::Location> where) {
    this->emplace_back(Diagnostic{Diagnostic::NOTE,
                                  std::move(message),
                                  std::move(where)});
  }
  void Diagnostics::emitWarning(std::string message, std::unique_ptr<Diagnostic::Location> where) {
    this->emplace_back(Diagnostic{Diagnostic::WARNING,
                                  std::move(message),
                                  std::move(where)});
  }
  void Diagnostics::emitError(std::string message, std::unique_ptr<Diagnostic::Location> where) {
    this->emplace_back(Diagnostic{Diagnostic::ERROR,
                                  std::move(message),
                                  std::move(where)});
  }
  void Diagnostics::emitInternalError(std::string message, std::unique_ptr<Diagnostic::Location> where) {
    this->emplace_back(Diagnostic{Diagnostic::INTERNAL_ERROR,
                                  std::move(message),
                                  std::move(where)});
  }
}  // namespace fluir
