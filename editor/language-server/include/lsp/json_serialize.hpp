#ifndef FLUIR_LSP_JSON_SERIALIZE_HPP
#define FLUIR_LSP_JSON_SERIALIZE_HPP

#include <nlohmann/json.hpp>

#include "lsp/api/document.hpp"
#include "lsp/api/language.hpp"
#include "lsp/api/lifecycle.hpp"

namespace fluir::lsp {
  // Lifecycle
  nlohmann::json toJson(const api::InitResponse&);
  nlohmann::json toJson(const api::ShutdownResponse&);
  // Document
  nlohmann::json toJson(const api::OpenDocResponse&);
  nlohmann::json toJson(const api::CloseDocResponse&);
  nlohmann::json toJson(const api::DocEditResponse&);
  // Language
  nlohmann::json toJson(const api::DocumentSymbol&);
  nlohmann::json toJson(const api::TaggedDocumentSymbol&);
  nlohmann::json toJson(const api::DocumentSymbols&);
  nlohmann::json toJson(const api::CompletionOption&);
  nlohmann::json toJson(const api::CompletionPossibilities&);
  nlohmann::json toJson(const api::SelectedCompletion&);
  nlohmann::json toJson(const api::ModuleDiagnostic&);
  nlohmann::json toJson(const api::Diagnostics&);
  // Primary template — declared but not defined
  template <typename T>
  T fromJson(const nlohmann::json&) = delete;

  // Lifecycle
  template <>
  api::InitRequest fromJson<api::InitRequest>(const nlohmann::json&);
  template <>
  api::ShutdownRequest fromJson<api::ShutdownRequest>(const nlohmann::json&);
  // Document
  template <>
  api::OpenDocRequest fromJson<api::OpenDocRequest>(const nlohmann::json&);
  template <>
  api::CloseDocRequest fromJson<api::CloseDocRequest>(const nlohmann::json&);
  template <>
  api::DocEdit fromJson<api::DocEdit>(const nlohmann::json&);
  // Language
  template <>
  api::DocumentSymbolRequest fromJson<api::DocumentSymbolRequest>(const nlohmann::json&);
  template <>
  api::CompletionsRequest fromJson<api::CompletionsRequest>(const nlohmann::json&);
  template <>
  api::SelectCompletion fromJson<api::SelectCompletion>(const nlohmann::json&);
  template <>
  api::RequestDiagnostics fromJson<api::RequestDiagnostics>(const nlohmann::json&);
}  // namespace fluir::lsp

#endif
