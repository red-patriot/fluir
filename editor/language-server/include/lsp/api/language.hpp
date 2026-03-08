#ifndef FLUIR_LSP_MODELS_LANGUAGE_HPP
#define FLUIR_LSP_MODELS_LANGUAGE_HPP

/** Public types for the language requests and responses of the LSP API */

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <compiler/models/id.hpp>
#include <compiler/models/location.hpp>

namespace fluir::lsp::api {
  struct DocumentSymbolRequest {
    std::string path;
  };

  struct DocumentSymbol {
    std::string name;
    std::optional<std::string> detail;
    std::optional<std::string> outType;
    std::optional<std::vector<std::string> > inType;
  };

  struct TaggedDocumentSymbol {
    FullID id;
    DocumentSymbol symbol;
  };

  struct DocumentSymbols {
    std::vector<TaggedDocumentSymbol> symbols;
  };

  struct CompletionsRequest {
    enum class Context {
      BODY,
      HEADER,
    };

    FullID parentBlock;
    Context context;
  };

  enum class CompletionKind {
    FUNCTION_DEF = 1,
    CALL = 2,
    CONSTANT = 3,
    OPERATOR = 4,
  };

  struct CompletionOption {
    std::string label;
    CompletionKind kind;
    std::optional<std::string> detail;
  };

  struct CompletionPossibilities {
    bool isComplete;
    std::vector<CompletionOption> completions;
  };

  struct SelectCompletion {
    FullID parentBlock;
    std::string selected;
  };

  struct SelectedCompletion {
    std::string text;
  };

  struct RequestDiagnostics {
    std::string path;
  };

  enum class DiagnosticSeverity {
    ERROR = 1,
    WARNING = 2,
    INFORMATION = 3,
    HINT = 4,
  };

  struct ModuleDiagnostic {
    std::variant<FullID, FlowGraphLocation> location;
    DiagnosticSeverity severity;
    std::string message;
  };

  struct Diagnostics {
    std::vector<ModuleDiagnostic> diagnostics;
  };
}  // namespace fluir::lsp::api

#endif
