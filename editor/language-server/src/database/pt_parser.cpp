#include "lsp/database/pt_parser.hpp"

#include <compiler/frontend/parser.hpp>
#include <compiler/types/builtin_symbols.hpp>
#include <compiler/utility/context.hpp>
#include <spdlog/spdlog.h>

#include "bytecode/version.hpp"
#include "lsp/database/compiler_diagnostics_shim.hpp"

namespace fluir::lsp {

  ParseResult parse(const CompilerOptions& options, const std::filesystem::path& file, const std::string& source) {
    ParseResult result;
    CompilerDiagnosticsShim sink{result.diagnostics};
    // TODO: Take this as a parameter
    Context ctx{.diagnosticSink = sink,
                .symbolTable = types::buildSymbolTable(),
                .currentFile = file,
                .outputFilename = file,
                .version = CURRENT_VERSION,
                .ignoreVersionChecks = options.developerOptions.suppressVersionErrors};

    auto tree = fluir::parseString(ctx, source);
    if (tree) {
      result.tree = std::move(*tree);
    }

    return result;
  }

}  // namespace fluir::lsp
