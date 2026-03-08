#ifndef FLUIR_LSP_JSON_SERIALIZE_HPP
#define FLUIR_LSP_JSON_SERIALIZE_HPP

#include <nlohmann/json.hpp>

#include "lsp/api/document.hpp"
#include "lsp/api/language.hpp"
#include "lsp/api/lifecycle.hpp"

namespace fluir::lsp {
  // Lifecycle
  nlohmann::json toJson(const api::InitRequest&);
  nlohmann::json toJson(const api::InitResponse&);
  nlohmann::json toJson(const api::ShutdownRequest&);
  nlohmann::json toJson(const api::ShutdownResponse&);
  // Document
  nlohmann::json toJson(const api::OpenDocRequest&);
  nlohmann::json toJson(const api::OpenDocResponse&);
  nlohmann::json toJson(const api::CloseDocRequest&);
  nlohmann::json toJson(const api::CloseDocResponse&);
  nlohmann::json toJson(const api::DocEdit&);
  nlohmann::json toJson(const api::DocEditResponse&);
  // Language
  nlohmann::json toJson(const api::DocumentSymbolRequest&);
  nlohmann::json toJson(const api::DocumentSymbol&);
  nlohmann::json toJson(const api::TaggedDocumentSymbol&);
  nlohmann::json toJson(const api::DocumentSymbols&);
  nlohmann::json toJson(const api::CompletionsRequest&);
  nlohmann::json toJson(const api::CompletionOption&);
  nlohmann::json toJson(const api::CompletionPossibilities&);
  nlohmann::json toJson(const api::SelectCompletion&);
  nlohmann::json toJson(const api::SelectedCompletion&);
}  // namespace fluir::lsp

#endif
