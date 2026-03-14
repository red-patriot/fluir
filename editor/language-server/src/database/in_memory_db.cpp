#include "lsp/database/in_memory_db.hpp"

#include <span>

#include <compiler/frontend/ast_builder.hpp>
#include <compiler/frontend/parser.hpp>
#include <compiler/frontend/type_checker.hpp>
#include <compiler/models/ast.hpp>
#include <compiler/models/ast/node.hpp>
#include <compiler/types/builtin_symbols.hpp>

#include "lsp/database/compiler_diagnostics_shim.hpp"

namespace fluir::lsp {

  namespace {
    std::string typeName(const types::SymbolTable& table, types::TypeID id) {
      auto* t = table.getType(id);
      return t ? t->name() : "?";
    }

    void collectSymbols(const ast::Node& node,
                        const types::SymbolTable& table,
                        std::span<const ast::FunctionDecl> declarations,
                        std::unordered_map<ID, api::Symbol>& symbols) {
      if (node.is<ast::Constant>()) {
        symbols[node.id()] = api::Symbol{
          .name = typeName(table, node.type()) + " Constant",
          .detail = std::nullopt,
          .outType = typeName(table, node.type()),
          .inType = std::nullopt,
        };
      } else if (node.is<ast::BinaryOp>()) {
        auto* bin = node.as<ast::BinaryOp>();
        symbols[node.id()] = api::Symbol{
          .name = "binary " + std::string(stringify(bin->op())),
          .detail = std::nullopt,
          .outType = std::nullopt,
          .inType = std::nullopt,
        };
        collectSymbols(*bin->lhs(), table, declarations, symbols);
        collectSymbols(*bin->rhs(), table, declarations, symbols);
      } else if (node.is<ast::UnaryOp>()) {
        auto* unary = node.as<ast::UnaryOp>();
        symbols[node.id()] = api::Symbol{
          .name = "unary " + std::string(stringify(unary->op())),
          .detail = std::nullopt,
          .outType = std::nullopt,
          .inType = std::nullopt,
        };
        collectSymbols(*unary->operand(), table, declarations, symbols);
      } else if (node.is<ast::Call>()) {
        auto* call = node.as<ast::Call>();
        std::optional<std::vector<std::string>> callInType;
        std::optional<std::string> callOutType;
        for (const auto& decl : declarations) {
          if (decl.name == call->target()) {
            if (!decl.parameters.empty()) {
              std::vector<std::string> paramTypes;
              for (const auto& p : decl.parameters) {
                paramTypes.push_back(p.typeName);
              }
              callInType = std::move(paramTypes);
            }
            if (decl.returnValue) {
              callOutType = decl.returnValue->typeName;
            }
            break;
          }
        }
        symbols[node.id()] = api::Symbol{
          .name = call->target(),
          .detail = std::nullopt,
          .outType = std::move(callOutType),
          .inType = std::move(callInType),
        };
        for (const auto& arg : call->arguments()) {
          collectSymbols(*arg, table, declarations, symbols);
        }
      } else if (node.is<ast::LocalWrite>()) {
        collectSymbols(*node.as<ast::LocalWrite>()->child(), table, declarations, symbols);
      } else if (node.is<ast::Cast>()) {
        collectSymbols(*node.as<ast::Cast>()->operand(), table, declarations, symbols);
      }
      // LocalRead: skip (compiler artifact)
    }

    DeclarationInfo buildDeclarationInfo(const ast::FunctionDecl& decl,
                                         const types::SymbolTable& table,
                                         std::span<const ast::FunctionDecl> declarations) {
      DeclarationInfo info;

      // Build the declaration's own symbol
      std::optional<std::vector<std::string>> inType;
      if (!decl.parameters.empty()) {
        std::vector<std::string> paramTypes;
        for (const auto& p : decl.parameters) {
          paramTypes.push_back(p.typeName);
        }
        inType = std::move(paramTypes);
      }
      std::optional<std::string> outType;
      if (decl.returnValue) {
        outType = decl.returnValue->typeName;
      }
      info.symbol = api::Symbol{
        .name = "func " + decl.name,
        .detail = std::nullopt,
        .outType = std::move(outType),
        .inType = std::move(inType),
      };

      // Walk statements to collect visible nodes
      for (const auto& stmt : decl.statements) {
        collectSymbols(*stmt, table, declarations, info.symbols);
      }

      return info;
    }
  }  // namespace

  InMemoryDB::InMemoryDB(const fluir::CompilerOptions& compilerOpts) : options_(compilerOpts) { }

  void InMemoryDB::setFileContents(std::filesystem::path file, std::string contents) {
    if (files_.contains(file)) {
      invalidate(file);
    }

    files_.insert_or_assign(file, FileState{});
    auto& fileState = files_[file];
    fileState.contents = std::move(contents);
    CompilerDiagnosticsShim sink{fileState.diagnostics};
    Context compilerContext{.diagnosticSink = sink,
                            .symbolTable = types::buildSymbolTable(),
                            .currentFile = file,
                            .outputFilename = file,
                            .version = CURRENT_VERSION,
                            .ignoreVersionChecks = options_.developerOptions.suppressVersionErrors};

    auto parseResults = fluir::parseString(compilerContext, fileState.contents);
    if (!parseResults) {
      return;
    }

    auto astResults = fluir::buildGraph(compilerContext, *parseResults);
    if (!astResults) {
      return;
    }

    auto typeCheckResults = fluir::typeCheck(compilerContext, std::move(*astResults));
    if (!typeCheckResults) {
      return;
    }

    // Build the declaration tree from compiler output
    for (const auto& decl : typeCheckResults->declarations) {
      fileState.declarations.emplace(
        decl.id, buildDeclarationInfo(decl, compilerContext.symbolTable, typeCheckResults->declarations));
    }

    files_[file] = std::move(fileState);
  }

  void InMemoryDB::invalidate(const std::filesystem::path& file) {
    if (files_.contains(file)) {
      files_.erase(file);
    }
  }

  std::span<const api::ModuleDiagnostic> InMemoryDB::diagnostics(const std::filesystem::path& file) {
    if (!files_.contains(file)) {
      throw std::runtime_error("Database does not contain the given file");
    }

    return files_.at(file).diagnostics;
  }

  std::optional<api::Symbol> InMemoryDB::symbolAt(const std::filesystem::path& file, FullID target) {
    if (!files_.contains(file) || target.empty()) {
      return std::nullopt;
    }

    const auto& fileState = files_.at(file);
    ID declId = target[0];

    auto declIt = fileState.declarations.find(declId);
    if (declIt == fileState.declarations.end()) {
      return std::nullopt;
    }

    if (target.size() == 1) {
      return declIt->second.symbol;
    }

    ID symbolId = target[1];
    if (symbolId == declId) {
      return declIt->second.symbol;
    }

    auto symIt = declIt->second.symbols.find(symbolId);
    if (symIt != declIt->second.symbols.end()) {
      return symIt->second;
    }

    return std::nullopt;
  }

}  // namespace fluir::lsp
