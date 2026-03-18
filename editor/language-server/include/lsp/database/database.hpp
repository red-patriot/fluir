#ifndef FLUIR_LSP_DATABASE_DATABASE_HPP
#define FLUIR_LSP_DATABASE_DATABASE_HPP

#include <filesystem>
#include <span>
#include <string>

#include <compiler/models/id.hpp>

#include "lsp/api/language.hpp"

namespace fluir::lsp {
  /** A generic database to provide completions for Fluir files*/
  class LanguageDatabase {
   public:
    virtual ~LanguageDatabase() = default;

    /** Sets the contents of the given file to the given string
     * Parses the contents for symbols and language intelligence
     */
    virtual void setFileContents(std::filesystem::path file, std::string contents) = 0;
    // TODO: Add incremental updates
    /** clears all cached queries for file */
    virtual void invalidate(const std::filesystem::path& file) = 0;

    /** Gets the diagnostics of the given file */
    virtual std::span<const api::ModuleDiagnostic> diagnostics(const std::filesystem::path& file) = 0;
    /** Gets the symbol at the given file and ID */
    virtual std::optional<api::Symbol> symbolAt(const std::filesystem::path& file, FullID target) = 0;
    /** Gets all symbols in the given file */
    virtual std::optional<std::vector<api::TaggedSymbol>> allSymbols(const std::filesystem::path& file) = 0;
    // TODO: Implement these:
    /** Gets the body completions at the given file and ID */
    // virtual api::CompletionPossibilities completionsInBody(const std::filesystem::path& file, FullID target) = 0;
    /** Gets the header completions at the given file and ID */
    // virtual api::CompletionPossibilities completionsInHeader(const std::filesystem::path& file, FullID target) = 0;
  };
}  // namespace fluir::lsp

#endif
