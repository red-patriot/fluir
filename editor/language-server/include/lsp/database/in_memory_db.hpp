#ifndef FLUIR_LSP_DATABASE_IN_MEMORY_DB_HPP
#define FLUIR_LSP_DATABASE_IN_MEMORY_DB_HPP

#include <filesystem>
#include <span>
#include <string>
#include <unordered_map>

#include <compiler/models/id.hpp>
#include <compiler/utility/options.hpp>

#include "lsp/api/language.hpp"

namespace fluir::lsp {

  struct DeclarationInfo {
    api::Symbol symbol;
    std::unordered_map<ID, api::Symbol> symbols;
  };

  /** A simple analysis database that stores data in memory */
  class InMemoryDB {
   public:
    explicit InMemoryDB(const fluir::CompilerOptions& compilerOpts);

    // --- Inputs (set from outside) ---
    void setFileContents(std::filesystem::path file, std::string contents);

    // --- Queries ---
    std::span<const api::ModuleDiagnostic> diagnostics(const std::filesystem::path& file);
    std::optional<api::Symbol> symbolAt(const std::filesystem::path& file, FullID target);
    api::CompletionPossibilities completionsInBody(const std::filesystem::path& file, FullID target);
    api::CompletionPossibilities completionsInHeader(const std::filesystem::path& file, FullID target);

    void invalidate(const std::filesystem::path& file);  // clears all cached queries for file

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
