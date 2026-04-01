#include "lsp/database/pt_analyzer.hpp"

#include <algorithm>

#include <spdlog/spdlog.h>

#include "lsp/database/pt_parser.hpp"
#include "lsp/database/pt_symbol_extractor.hpp"

namespace fluir::lsp {

  PtAnalyzer::PtAnalyzer(const fluir::CompilerOptions& compilerOpts) : options_(compilerOpts) { }

  void PtAnalyzer::setFileContents(std::filesystem::path file, std::string contents) {
    spdlog::debug("PtAnalyzer: Analyzing file: {}", file.string());
    if (files_.contains(file)) {
      invalidate(file);
    }

    auto [it, inserted] = files_.insert_or_assign(file, FileState{});
    auto& [path, fileState] = *it;
    fileState.contents = std::move(contents);

    // Subcomponent 1: Parse
    auto parseResult = parse(options_, file, fileState.contents);

    // Merge parse diagnostics
    fileState.diagnostics = std::move(parseResult.diagnostics);

    if (!parseResult.tree) {
      spdlog::warn("PtAnalyzer: Parse failed for: {}", file.string());
      return;
    }

    // Subcomponent 2: Extract symbols
    auto extraction = extractSymbols(*parseResult.tree, file);

    // Merge extraction diagnostics
    for (auto& diag : extraction.diagnostics) {
      fileState.diagnostics.push_back(std::move(diag));
    }

    fileState.declarations = std::move(extraction.declarations);
  }

  void PtAnalyzer::invalidate(const std::filesystem::path& file) {
    spdlog::debug("PtAnalyzer: Invalidating file: {}", file.string());
    if (files_.contains(file)) {
      files_.erase(file);
    }
  }

  std::span<const api::ModuleDiagnostic> PtAnalyzer::diagnostics(const std::filesystem::path& file) {
    if (!files_.contains(file)) {
      spdlog::warn("PtAnalyzer: Diagnostics requested for unknown file: {}", file.string());
      throw std::runtime_error("Database does not contain the given file");
    }
    return files_.at(file).diagnostics;
  }

  std::optional<api::Symbol> PtAnalyzer::symbolAt(const std::filesystem::path& file, FullID target) {
    if (!files_.contains(file) || target.empty()) {
      return std::nullopt;
    }

    const auto& fileState = files_.at(file);
    // TODO: Support deeper nesting of symbols
    ID declId = target.front();

    auto declIt = fileState.declarations.find(declId);
    if (declIt == fileState.declarations.end()) {
      return std::nullopt;
    }

    if (target.size() == 1) {
      return declIt->second.symbol;
    }

    ID symbolId = target.back();
    if (symbolId == declId) {
      return declIt->second.symbol;
    }

    auto symIt = declIt->second.symbols.find(symbolId);
    if (symIt != declIt->second.symbols.end()) {
      return symIt->second;
    }

    return std::nullopt;
  }

  std::optional<std::vector<api::TaggedSymbol>> PtAnalyzer::allSymbols(const std::filesystem::path& file) {
    if (!files_.contains(file)) {
      return std::nullopt;
    }

    const auto& fileState = files_.at(file);
    std::vector<api::TaggedSymbol> result;

    for (const auto& [declId, declInfo] : fileState.declarations) {
      result.push_back(api::TaggedSymbol{.id = {declId}, .symbol = declInfo.symbol});

      for (const auto& [symbolId, symbol] : declInfo.symbols) {
        result.push_back(api::TaggedSymbol{.id = {declId, symbolId}, .symbol = symbol});
      }
    }

    // This is just to make tests pass for now
    // TODO: Remove extra sorting
    std::ranges::sort(result, [](const auto& a, const auto& b) { return a.id < b.id; });
    return result;
  }

}  // namespace fluir::lsp
