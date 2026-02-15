#ifndef FLUIR_COMPILER_UTILITY_CONTEXT_HPP
#define FLUIR_COMPILER_UTILITY_CONTEXT_HPP

#include <filesystem>

#include "bytecode/version.hpp"
#include "compiler/types/symbol_table.hpp"
#include "compiler/utility/diagnostic/ostream_sink.hpp"
#include "compiler/utility/diagnostics.hpp"
#include "compiler/utility/options.hpp"
#include "compiler/utility/results.hpp"

namespace fluir {
  /** The context of compilation. */
  struct Context {
    diagnostic::Sink& diagnosticSink{diagnostic::getCoutSink()}; /**< The sink to emit diagnostics to */
    types::SymbolTable symbolTable;                              /**< The symbol table of the compilation */
    std::filesystem::path currentFile;                           /**< The current file being processed */
    std::filesystem::path outputFilename;                        /**< The output filename to write to */
    Version version{0, 0, 0};                                    /**< The current version of the compiler */
  };
}  // namespace fluir

#endif
