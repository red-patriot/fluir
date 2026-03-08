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
  // Primary template — declared but not defined
  template <typename T>
  T fromJson(const nlohmann::json&) = delete;

  // Lifecycle
  template <>
  api::InitRequest fromJson<api::InitRequest>(const nlohmann::json&);
  template <>
  api::InitResponse fromJson<api::InitResponse>(const nlohmann::json&);
  template <>
  api::ShutdownRequest fromJson<api::ShutdownRequest>(const nlohmann::json&);
  template <>
  api::ShutdownResponse fromJson<api::ShutdownResponse>(const nlohmann::json&);
  // Document
  template <>
  api::OpenDocRequest fromJson<api::OpenDocRequest>(const nlohmann::json&);
  template <>
  api::OpenDocResponse fromJson<api::OpenDocResponse>(const nlohmann::json&);
  template <>
  api::CloseDocRequest fromJson<api::CloseDocRequest>(const nlohmann::json&);
  template <>
  api::CloseDocResponse fromJson<api::CloseDocResponse>(const nlohmann::json&);
  template <>
  api::DocEdit fromJson<api::DocEdit>(const nlohmann::json&);
  template <>
  api::DocEditResponse fromJson<api::DocEditResponse>(const nlohmann::json&);
  // Language
  template <>
  api::DocumentSymbolRequest fromJson<api::DocumentSymbolRequest>(const nlohmann::json&);
  template <>
  api::DocumentSymbol fromJson<api::DocumentSymbol>(const nlohmann::json&);
  template <>
  api::TaggedDocumentSymbol fromJson<api::TaggedDocumentSymbol>(const nlohmann::json&);
  template <>
  api::DocumentSymbols fromJson<api::DocumentSymbols>(const nlohmann::json&);
  template <>
  api::CompletionsRequest fromJson<api::CompletionsRequest>(const nlohmann::json&);
  template <>
  api::CompletionOption fromJson<api::CompletionOption>(const nlohmann::json&);
  template <>
  api::CompletionPossibilities fromJson<api::CompletionPossibilities>(const nlohmann::json&);
  template <>
  api::SelectCompletion fromJson<api::SelectCompletion>(const nlohmann::json&);
  template <>
  api::SelectedCompletion fromJson<api::SelectedCompletion>(const nlohmann::json&);
}  // namespace fluir::lsp

#endif
