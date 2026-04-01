#ifndef FLUIR_LSP_DATABASE_PT_ANALYZER_HPP
#define FLUIR_LSP_DATABASE_PT_ANALYZER_HPP

#include <unordered_map>

#include <compiler/utility/options.hpp>

#include "lsp/database/database.hpp"
#include "lsp/database/pt_symbol_extractor.hpp"

namespace fluir::lsp {
  class PtAnalyzer : public LanguageDatabase {
   public:
    explicit PtAnalyzer(const fluir::CompilerOptions& compilerOpts);

    void setFileContents(std::filesystem::path file, std::string contents) override;
    void invalidate(const std::filesystem::path& file) override;

    std::span<const api::ModuleDiagnostic> diagnostics(const std::filesystem::path& file) override;
    std::optional<api::Symbol> symbolAt(const std::filesystem::path& file, FullID target) override;
    std::optional<std::vector<api::TaggedSymbol>> allSymbols(const std::filesystem::path& file) override;

   private:
    struct FileState {
      std::string contents;
      std::unordered_map<ID, DeclarationInfo> declarations;
      std::vector<api::ModuleDiagnostic> diagnostics;
    };
    CompilerOptions options_;
    std::unordered_map<std::filesystem::path, FileState> files_{};
  };
}  // namespace fluir::lsp

#endif
