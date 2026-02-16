#include <cerrno>
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
#include "compiler/utility/diagnostic/colored_stdout_sink.hpp"
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

int main(int argc, const char** argv) {
  auto options = fluir::parseArgs(argc, argv);
  if (!options) {
    // Some arguments were invalid, report this in a POSIX-compliant manner
    return EINVAL;
  }

  try {
    fluir::diagnostic::Sink& sink = [&]() -> fluir::diagnostic::Sink& {
      if (options->colorOutput) {
        return fluir::diagnostic::getColoredStdoutSink();
      }
      return fluir::diagnostic::getCoutSink();
    }();

    // TODO: Select the correct diagnostics sink based on options
    fluir::Context ctx{
      .diagnosticSink = sink,
      .symbolTable = fluir::types::buildSymbolTable(),
      // This is for the future when we take in multiple input files
      .currentFile = fs::canonical(options->inputFilename),
      .outputFilename = options->outputFilename,
      .version = fluir::CURRENT_VERSION,
    };

    auto ast = runFrontend(ctx, ctx.currentFile);
    if (!ast) {
      return EXIT_FAILURE;
    }

    auto backendResults = fluir::generateCode(ctx, *ast);
    if (!backendResults) {
      return EXIT_FAILURE;
    }

    {
      std::ofstream fout{ctx.outputFilename};
      fluir::InspectWriter writer{};
      fluir::writeCode(*backendResults, writer, fout);
    }

    return EXIT_SUCCESS;
  } catch (const std::filesystem::filesystem_error& e) {
    std::cerr << "" << e.what() << '\n';
    return EINVAL;
  } catch (const std::exception& e) {
    std::cerr << "An internal error occurred:\n\t" << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
