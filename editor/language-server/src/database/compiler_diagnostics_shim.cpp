#include "lsp/database/compiler_diagnostics_shim.hpp"

#include <compiler/utility/diagnostic/pretty_msg.hpp>

namespace fluir::lsp {

  CompilerDiagnosticsShim::CompilerDiagnosticsShim(std::vector<api::ModuleDiagnostic>& diagnostics) :
    diagnostics_(&diagnostics) { }

  void CompilerDiagnosticsShim::report(diagnostic::Code code,
                                       const std::filesystem::path& /*file*/,
                                       const ErrorLocation& location,
                                       std::string_view extraMsg) {
    auto severity = api::DiagnosticSeverity::INFORMATION;
    if (code >= diagnostic::Code::GENERIC_ERROR) {
      severity = api::DiagnosticSeverity::ERROR;
    } else if (code >= diagnostic::Code::GENERIC_WARNING) {
      severity = api::DiagnosticSeverity::WARNING;
    }

    auto loc = std::visit(
      [](const auto& v) -> std::variant<FullID, FlowGraphLocation> {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, FullID>) {
          return v;
        } else {
          return FullID{};
        }
      },
      location);

    auto message = diagnostic::prettyMessage(code);
    if (!extraMsg.empty()) {
      message += ": ";
      message += extraMsg;
    }

    diagnostics_->push_back(api::ModuleDiagnostic{
      .location = std::move(loc),
      .severity = severity,
      .message = std::move(message),
    });
  }

}  // namespace fluir::lsp
