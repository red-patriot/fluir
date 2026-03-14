#ifndef FLUIR_LSP_DATABASE_COMPILER_DIAGNOSTICS_SHIM_HPP
#define FLUIR_LSP_DATABASE_COMPILER_DIAGNOSTICS_SHIM_HPP

#include <compiler/utility/diagnostic/sink.hpp>

namespace fluir::lsp {
  class CompilerDiagnosticsShim : public diagnostic::Sink {
   private:
    void report(diagnostic::Code, const std::filesystem::path&, const ErrorLocation&, std::string_view) override {
      // TODO
    }
  };
}  // namespace fluir::lsp

#endif
