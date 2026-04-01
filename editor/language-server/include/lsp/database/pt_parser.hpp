#ifndef FLUIR_LSP_DATABASE_PT_PARSER_HPP
#define FLUIR_LSP_DATABASE_PT_PARSER_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <compiler/frontend/parse_tree/parse_tree.hpp>
#include <compiler/utility/options.hpp>

#include "lsp/api/language.hpp"

namespace fluir::lsp {
  struct ParseResult {
    std::optional<pt::ParseTree> tree;
    std::vector<api::ModuleDiagnostic> diagnostics;
  };

  ParseResult parse(const CompilerOptions& options, const std::filesystem::path& file, const std::string& source);
}  // namespace fluir::lsp

#endif
