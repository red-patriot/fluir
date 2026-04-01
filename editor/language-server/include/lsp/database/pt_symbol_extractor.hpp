#ifndef FLUIR_LSP_DATABASE_PT_SYMBOL_EXTRACTOR_HPP
#define FLUIR_LSP_DATABASE_PT_SYMBOL_EXTRACTOR_HPP

#include <filesystem>
#include <unordered_map>
#include <vector>

#include <compiler/frontend/parse_tree/parse_tree.hpp>
#include <compiler/models/id.hpp>

#include "lsp/api/language.hpp"

namespace fluir::lsp {
  struct DeclarationInfo {
    api::Symbol symbol;
    std::unordered_map<ID, api::Symbol> symbols;
  };

  struct ExtractionResult {
    std::unordered_map<ID, DeclarationInfo> declarations;
    std::vector<api::ModuleDiagnostic> diagnostics;
  };

  ExtractionResult extractSymbols(const pt::ParseTree& tree, const std::filesystem::path& file);
}  // namespace fluir::lsp

#endif
