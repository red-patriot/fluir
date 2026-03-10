#ifndef FLUIR_LSP_DATABASE_IN_MEMORY_DB_HPP
#define FLUIR_LSP_DATABASE_IN_MEMORY_DB_HPP

#include <filesystem>
#include <span>
#include <string>

#include <compiler/frontend/parse_tree/parse_tree.hpp>
#include <compiler/models/ast.hpp>
#include <compiler/types/symbol_table.hpp>

#include "lsp/api/language.hpp"

namespace fluir::lsp {

  /** A simple analysis database that stores data in memory */
  class InMemoryDB {
   public:
    // --- Inputs (set from outside) ---
    void setFileContents(std::filesystem::path file, std::string contents);

    // --- Queries (derived, cached) ---
    const fluir::pt::ParseTree& parsed(const std::filesystem::path& file);
    const fluir::ast::AST& resolved(const std::filesystem::path& file);
    const fluir::ast::AST& typechecked(const std::filesystem::path& file);

    // --- Convenience queries (built on top) ---
    std::span<const api::ModuleDiagnostic> diagnostics(const std::filesystem::path& file);
    std::vector<api::Symbol> symbolsAt(const std::filesystem::path& file, FullID target);
    api::CompletionPossibilities completionsInBody(const std::filesystem::path& file, FullID target);
    api::CompletionPossibilities completionsInHeader(const std::filesystem::path& file, FullID target);

    void invalidate(const std::filesystem::path& file);  // clears all cached queries for file

   private:
    struct FileState {
      std::string contents;
      std::optional<pt::ParseTree> parseCache;
      std::optional<ast::AST> resolveCache;
      std::optional<fluir::types::SymbolTable> typecheckCache;
    };
    std::unordered_map<std::filesystem::path, FileState> files_;
  };
}  // namespace fluir::lsp

#endif
