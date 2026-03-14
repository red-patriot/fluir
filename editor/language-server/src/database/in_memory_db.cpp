#include "lsp/database/in_memory_db.hpp"

#include <compiler/frontend/ast_builder.hpp>
#include <compiler/frontend/parser.hpp>
#include <compiler/frontend/type_checker.hpp>
#include <compiler/types/builtin_symbols.hpp>

#include "lsp/database/compiler_diagnostics_shim.hpp"

namespace fluir::lsp {
  InMemoryDB::InMemoryDB(const fluir::CompilerOptions& compilerOpts) : options_(compilerOpts) { }

  void InMemoryDB::setFileContents(std::filesystem::path file, std::string contents) {
    if (files_.contains(file)) {
      invalidate(file);
    }

    files_.insert_or_assign(file, FileState{});
    auto& fileState = files_[file];
    fileState.contents = std::move(contents);
    CompilerDiagnosticsShim sink;
    Context compilerContext{.diagnosticSink = sink,
                            .symbolTable = types::buildSymbolTable(),
                            .currentFile = file,
                            .outputFilename = file,
                            .version = CURRENT_VERSION,
                            .ignoreVersionChecks = options_.developerOptions.suppressVersionErrors};

    // TODO: Capture compiler outputs
    auto parseResults = fluir::parseString(compilerContext, fileState.contents);
    if (!parseResults) {
      // TODO: errors
      return;
    }
    fileState.parseCache = std::move(parseResults);
    // TODO: AST and type analysis
    auto astResults = fluir::buildGraph(compilerContext, *fileState.parseCache);
    if (!astResults) {
      // TODO: errors
      return;
    }

    auto typeCheckResults = fluir::typeCheck(compilerContext, std::move(*astResults));
    if (!typeCheckResults) {
      // TODO: errors
      return;
    }
    fileState.resolveCache = std::move(typeCheckResults);
    fileState.typecheckCache = std::move(compilerContext.symbolTable);

    files_[file] = std::move(fileState);
  }

  void InMemoryDB::invalidate(const std::filesystem::path& file) {
    if (files_.contains(file)) {
      files_.erase(file);
    }
  }

  const fluir::pt::ParseTree& InMemoryDB::parsed(const std::filesystem::path& file) {
    if (!files_.contains(file)) {
      throw std::runtime_error("Database does not contain the given file");
    }
    if (!files_.at(file).parseCache) {
      throw std::runtime_error("Given file could not be parsed");
    }

    return *files_.at(file).parseCache;
  }

  const fluir::ast::AST& InMemoryDB::resolved(const std::filesystem::path& file) {
    if (!files_.contains(file)) {
      throw std::runtime_error("Database does not contain the given file");
    }
    if (!files_.at(file).resolveCache) {
      throw std::runtime_error("Given file could not be resolved");
    }

    return *files_.at(file).resolveCache;
  }

  const ast::AST& InMemoryDB::typechecked(const std::filesystem::path& file) {
    if (!files_.contains(file)) {
      throw std::runtime_error("Database does not contain the given file");
    }
    if (!files_.at(file).typecheckCache || !files_.at(file).resolveCache) {
      throw std::runtime_error("Given file could not be type checked");
    }

    return *files_.at(file).resolveCache;
  }

}  // namespace fluir::lsp
