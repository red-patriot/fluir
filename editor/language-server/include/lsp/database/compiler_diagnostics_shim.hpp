#ifndef FLUIR_LSP_DATABASE_COMPILER_DIAGNOSTICS_SHIM_HPP
#define FLUIR_LSP_DATABASE_COMPILER_DIAGNOSTICS_SHIM_HPP

#include <vector>

#include <compiler/utility/diagnostic/sink.hpp>
#include <lsp/api/language.hpp>

namespace fluir::lsp {
  class CompilerDiagnosticsShim : public diagnostic::Sink {
   public:
    explicit CompilerDiagnosticsShim(std::vector<api::ModuleDiagnostic>& diagnostics);

   private:
    std::vector<api::ModuleDiagnostic>* diagnostics_;

    void report(diagnostic::Code code,
                const std::filesystem::path& file,
                const ErrorLocation& location,
                std::string_view extraMsg) override;
  };
}  // namespace fluir::lsp

#endif
