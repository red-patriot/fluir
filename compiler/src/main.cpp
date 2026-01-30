#include <filesystem>
#include <fstream>
#include <iostream>

#include "bytecode/version.hpp"
#include "compiler/backend/bytecode_generator.hpp"
#include "compiler/backend/inspect_writer.hpp"
#include "compiler/frontend/ast_builder.hpp"
#include "compiler/frontend/parser.hpp"
#include "compiler/frontend/type_checker.hpp"
#include "compiler/types/builtin_symbols.hpp"
#include "compiler/utility/context.hpp"
namespace fs = std::filesystem;

namespace {
  fluir::Results<fluir::ast::AST> runFrontend(fluir::Context& ctx, const fs::path& source) {
    auto parseTree = fluir::parseFile(ctx, source);
    if (!parseTree) {
      return fluir::NoResult;
    }
    auto ast = fluir::buildGraph(ctx, *parseTree);
    if (!ast) {
      return fluir::NoResult;
    }
    ast = fluir::typeCheck(ctx, std::move(*ast));
    if (!ast) {
      return fluir::NoResult;
    }
    return ast;
  }
}  // namespace

int main(int argc, char** argv) {
  // TODO: Read real inputs from the command line
  try {
    if (argc != 2) {
      std::cerr << "Usage: fluir.compiler file.fl\n";
      return 1;
    }
    fs::path source;
    try {
      source = fs::canonical(fs::path{argv[1]});
    } catch (const std::filesystem::filesystem_error& e) {
      std::cerr << e.what() << '\n';
      return 1;
    }
    fluir::Context ctx{.version = fluir::CURRENT_VERSION};
    ctx.symbolTable = fluir::types::buildSymbolTable();

    auto ast = runFrontend(ctx, source);
    if (!ast) {
      return 1;
    }

    auto backendResults = fluir::generateCode(ctx, *ast);
    if (!backendResults) {
      return 1;
    }

    {
      fs::path destination{"./out.flc"};
      std::ofstream fout{destination};
      fluir::InspectWriter writer{};
      fluir::writeCode(*backendResults, writer, fout);
    }

    return 0;
  } catch (const std::exception& e) {
    std::cerr << "An internal error occurred:\n\t" << e.what() << '\n';
    return 1;
  }
}
