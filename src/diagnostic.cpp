#include "fluir/compiler/diagnostic.hpp"

namespace fluir::compiler {
  std::ostream& operator<<(std::ostream& os, const Diagnostic& diagnostic) {
    os << fmt::format("[{}] {}: {}",
                      diagnostic.level,
                      diagnostic.where ? diagnostic.where->string() : "",
                      diagnostic.message);

    return os;
  }
}  // namespace fluir::compiler

auto fmt::formatter<fluir::compiler::Diagnostic::Level>::format(fluir::compiler::Diagnostic::Level l,
                                                                format_context& ctx) const -> fmt::format_context::iterator {
  using enum fluir::compiler::Diagnostic::Level;

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
      name = "INTERNAL_ERROR";
      break;
  }

  return formatter<fmt::string_view>::format(name, ctx);
}
