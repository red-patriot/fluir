#include "lsp/json_serialize.hpp"

namespace fluir::lsp {

  // --- Lifecycle ---

  nlohmann::json toJson(const api::InitRequest&) { return nlohmann::json::object(); }

  nlohmann::json toJson(const api::InitResponse& r) {
    auto obj = nlohmann::json::object();
    obj["version"] = r.version;
    return obj;
  }

  nlohmann::json toJson(const api::ShutdownRequest&) { return nlohmann::json::object(); }

  nlohmann::json toJson(const api::ShutdownResponse&) { return nlohmann::json::object(); }

  // --- Document ---

  nlohmann::json toJson(const api::OpenDocRequest& r) {
    auto obj = nlohmann::json::object();
    obj["path"] = r.path;
    obj["content"] = r.content;
    return obj;
  }

  nlohmann::json toJson(const api::OpenDocResponse&) { return nlohmann::json::object(); }

  nlohmann::json toJson(const api::CloseDocRequest& r) {
    auto obj = nlohmann::json::object();
    obj["path"] = r.path;
    return obj;
  }

  nlohmann::json toJson(const api::CloseDocResponse&) { return nlohmann::json::object(); }

  nlohmann::json toJson(const api::DocEdit& r) {
    auto obj = nlohmann::json::object();
    obj["contents"] = r.contents;
    return obj;
  }

  nlohmann::json toJson(const api::DocEditResponse&) { return nlohmann::json::object(); }

  // --- Language ---

  nlohmann::json toJson(const api::DocumentSymbolRequest& r) {
    auto obj = nlohmann::json::object();
    obj["path"] = r.path;
    return obj;
  }

  nlohmann::json toJson(const api::DocumentSymbol& s) {
    auto obj = nlohmann::json::object();
    obj["name"] = s.name;
    if (s.detail) {
      obj["detail"] = *s.detail;
    }
    if (s.outType) {
      obj["outType"] = *s.outType;
    }
    if (s.inType) {
      auto arr = nlohmann::json::array();
      for (const auto& t : *s.inType) {
        arr.push_back(t);
      }
      obj["inType"] = arr;
    }
    return obj;
  }

  nlohmann::json toJson(const api::TaggedDocumentSymbol& s) {
    auto obj = nlohmann::json::object();
    auto idArr = nlohmann::json::array();
    for (auto id : s.id) {
      idArr.push_back(id);
    }
    obj["id"] = idArr;
    obj["symbol"] = toJson(s.symbol);
    return obj;
  }

  nlohmann::json toJson(const api::DocumentSymbols& s) {
    auto obj = nlohmann::json::object();
    auto arr = nlohmann::json::array();
    for (const auto& sym : s.symbols) {
      arr.push_back(toJson(sym));
    }
    obj["symbols"] = arr;
    return obj;
  }

  nlohmann::json toJson(const api::CompletionsRequest& r) {
    auto obj = nlohmann::json::object();
    auto idArr = nlohmann::json::array();
    for (auto id : r.parentBlock) {
      idArr.push_back(id);
    }
    obj["parentBlock"] = idArr;
    obj["context"] = static_cast<int>(r.context);
    return obj;
  }

  nlohmann::json toJson(const api::CompletionOption& o) {
    auto obj = nlohmann::json::object();
    obj["label"] = o.label;
    obj["kind"] = static_cast<int>(o.kind);
    if (o.detail) {
      obj["detail"] = *o.detail;
    }
    return obj;
  }

  nlohmann::json toJson(const api::CompletionPossibilities& p) {
    auto obj = nlohmann::json::object();
    obj["isComplete"] = p.isComplete;
    auto arr = nlohmann::json::array();
    for (const auto& c : p.completions) {
      arr.push_back(toJson(c));
    }
    obj["completions"] = arr;
    return obj;
  }

  nlohmann::json toJson(const api::SelectCompletion& r) {
    auto obj = nlohmann::json::object();
    auto idArr = nlohmann::json::array();
    for (auto id : r.parentBlock) {
      idArr.push_back(id);
    }
    obj["parentBlock"] = idArr;
    obj["selected"] = r.selected;
    return obj;
  }

  nlohmann::json toJson(const api::SelectedCompletion& r) {
    auto obj = nlohmann::json::object();
    obj["text"] = r.text;
    return obj;
  }

  nlohmann::json toJson(const api::RequestDiagnostics& r) {
    auto obj = nlohmann::json::object();
    obj["path"] = r.path;
    return obj;
  }

  nlohmann::json toJson(const api::ModuleDiagnostic& m) {
    auto obj = nlohmann::json::object();
    std::visit(
      [&obj](const auto& loc) {
        using T = std::decay_t<decltype(loc)>;
        if constexpr (std::is_same_v<T, FullID>) {
          auto arr = nlohmann::json::array();
          for (auto id : loc) {
            arr.push_back(id);
          }
          obj["location"] = arr;
        } else {
          auto locObj = nlohmann::json::object();
          locObj["x"] = loc.x;
          locObj["y"] = loc.y;
          locObj["z"] = loc.z;
          locObj["width"] = loc.width;
          locObj["height"] = loc.height;
          obj["location"] = locObj;
        }
      },
      m.location);
    obj["severity"] = static_cast<int>(m.severity);
    obj["message"] = m.message;
    return obj;
  }

  nlohmann::json toJson(const api::Diagnostics& d) {
    auto obj = nlohmann::json::object();
    auto arr = nlohmann::json::array();
    for (const auto& diag : d.diagnostics) {
      arr.push_back(toJson(diag));
    }
    obj["diagnostics"] = arr;
    return obj;
  }

  // --- fromJson specializations ---

  // Lifecycle

  template <>
  api::InitRequest fromJson<api::InitRequest>(const nlohmann::json&) {
    return {};
  }

  template <>
  api::InitResponse fromJson<api::InitResponse>(const nlohmann::json& j) {
    return {j.at("version").get<std::string>()};
  }

  template <>
  api::ShutdownRequest fromJson<api::ShutdownRequest>(const nlohmann::json&) {
    return {};
  }

  template <>
  api::ShutdownResponse fromJson<api::ShutdownResponse>(const nlohmann::json&) {
    return {};
  }

  // Document

  template <>
  api::OpenDocRequest fromJson<api::OpenDocRequest>(const nlohmann::json& j) {
    return {j.at("path").get<std::string>(), j.at("content").get<std::string>()};
  }

  template <>
  api::OpenDocResponse fromJson<api::OpenDocResponse>(const nlohmann::json&) {
    return {};
  }

  template <>
  api::CloseDocRequest fromJson<api::CloseDocRequest>(const nlohmann::json& j) {
    return {j.at("path").get<std::string>()};
  }

  template <>
  api::CloseDocResponse fromJson<api::CloseDocResponse>(const nlohmann::json&) {
    return {};
  }

  template <>
  api::DocEdit fromJson<api::DocEdit>(const nlohmann::json& j) {
    return {j.at("contents").get<std::string>()};
  }

  template <>
  api::DocEditResponse fromJson<api::DocEditResponse>(const nlohmann::json&) {
    return {};
  }

  // Language

  template <>
  api::DocumentSymbolRequest fromJson<api::DocumentSymbolRequest>(const nlohmann::json& j) {
    return {j.at("path").get<std::string>()};
  }

  template <>
  api::DocumentSymbol fromJson<api::DocumentSymbol>(const nlohmann::json& j) {
    std::optional<std::string> detail =
      j.contains("detail") ? std::optional{j.at("detail").get<std::string>()} : std::nullopt;
    std::optional<std::string> outType =
      j.contains("outType") ? std::optional{j.at("outType").get<std::string>()} : std::nullopt;
    std::optional<std::vector<std::string>> inType =
      j.contains("inType") ? std::optional{j.at("inType").get<std::vector<std::string>>()} : std::nullopt;
    return {j.at("name").get<std::string>(), detail, outType, inType};
  }

  template <>
  api::TaggedDocumentSymbol fromJson<api::TaggedDocumentSymbol>(const nlohmann::json& j) {
    FullID id = j.at("id").get<std::vector<uint64_t>>();
    return {id, fromJson<api::DocumentSymbol>(j.at("symbol"))};
  }

  template <>
  api::DocumentSymbols fromJson<api::DocumentSymbols>(const nlohmann::json& j) {
    std::vector<api::TaggedDocumentSymbol> symbols;
    for (const auto& elem : j.at("symbols")) {
      symbols.push_back(fromJson<api::TaggedDocumentSymbol>(elem));
    }
    return {symbols};
  }

  template <>
  api::CompletionsRequest fromJson<api::CompletionsRequest>(const nlohmann::json& j) {
    FullID parentBlock = j.at("parentBlock").get<std::vector<uint64_t>>();
    auto context = static_cast<api::CompletionsRequest::Context>(j.at("context").get<int>());
    return {parentBlock, context};
  }

  template <>
  api::CompletionOption fromJson<api::CompletionOption>(const nlohmann::json& j) {
    std::optional<std::string> detail =
      j.contains("detail") ? std::optional{j.at("detail").get<std::string>()} : std::nullopt;
    return {j.at("label").get<std::string>(), static_cast<api::CompletionKind>(j.at("kind").get<int>()), detail};
  }

  template <>
  api::CompletionPossibilities fromJson<api::CompletionPossibilities>(const nlohmann::json& j) {
    std::vector<api::CompletionOption> completions;
    for (const auto& elem : j.at("completions")) {
      completions.push_back(fromJson<api::CompletionOption>(elem));
    }
    return {j.at("isComplete").get<bool>(), completions};
  }

  template <>
  api::SelectCompletion fromJson<api::SelectCompletion>(const nlohmann::json& j) {
    FullID parentBlock = j.at("parentBlock").get<std::vector<uint64_t>>();
    return {parentBlock, j.at("selected").get<std::string>()};
  }

  template <>
  api::SelectedCompletion fromJson<api::SelectedCompletion>(const nlohmann::json& j) {
    return {j.at("text").get<std::string>()};
  }

  template <>
  api::RequestDiagnostics fromJson<api::RequestDiagnostics>(const nlohmann::json& j) {
    return {j.at("path").get<std::string>()};
  }

  template <>
  api::ModuleDiagnostic fromJson<api::ModuleDiagnostic>(const nlohmann::json& j) {
    const auto& loc = j.at("location");
    std::variant<FullID, FlowGraphLocation> location;
    if (loc.is_array()) {
      location = loc.get<FullID>();
    } else {
      location = FlowGraphLocation{
        loc.at("x").get<int>(),
        loc.at("y").get<int>(),
        loc.at("z").get<int>(),
        loc.at("width").get<int>(),
        loc.at("height").get<int>(),
      };
    }
    auto severity = static_cast<api::DiagnosticSeverity>(j.at("severity").get<int>());
    return {location, severity, j.at("message").get<std::string>()};
  }

  template <>
  api::Diagnostics fromJson<api::Diagnostics>(const nlohmann::json& j) {
    std::vector<api::ModuleDiagnostic> diagnostics;
    for (const auto& elem : j.at("diagnostics")) {
      diagnostics.push_back(fromJson<api::ModuleDiagnostic>(elem));
    }
    return {diagnostics};
  }

}  // namespace fluir::lsp
