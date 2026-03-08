#include "lsp/json_serialize.hpp"

namespace fluir::lsp {

  // --- Lifecycle ---

  nlohmann::json toJson(const api::InitResponse& r) {
    auto obj = nlohmann::json::object();
    obj["version"] = r.version;
    return obj;
  }

  nlohmann::json toJson(const api::ShutdownResponse&) { return nlohmann::json::object(); }

  // --- Document ---

  nlohmann::json toJson(const api::OpenDocResponse&) { return nlohmann::json::object(); }

  nlohmann::json toJson(const api::CloseDocResponse&) { return nlohmann::json::object(); }

  nlohmann::json toJson(const api::DocEditResponse&) { return nlohmann::json::object(); }

  // --- Language ---

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

  nlohmann::json toJson(const api::SelectedCompletion& r) {
    auto obj = nlohmann::json::object();
    obj["text"] = r.text;
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
  api::ShutdownRequest fromJson<api::ShutdownRequest>(const nlohmann::json&) {
    return {};
  }

  // Document

  template <>
  api::OpenDocRequest fromJson<api::OpenDocRequest>(const nlohmann::json& j) {
    return {j.at("path").get<std::string>(), j.at("content").get<std::string>()};
  }

  template <>
  api::CloseDocRequest fromJson<api::CloseDocRequest>(const nlohmann::json& j) {
    return {j.at("path").get<std::string>()};
  }

  template <>
  api::DocEdit fromJson<api::DocEdit>(const nlohmann::json& j) {
    return {j.at("contents").get<std::string>()};
  }

  // Language

  template <>
  api::DocumentSymbolRequest fromJson<api::DocumentSymbolRequest>(const nlohmann::json& j) {
    return {j.at("path").get<std::string>()};
  }

  template <>
  api::CompletionsRequest fromJson<api::CompletionsRequest>(const nlohmann::json& j) {
    FullID parentBlock = j.at("parentBlock").get<std::vector<uint64_t>>();
    auto context = static_cast<api::CompletionsRequest::Context>(j.at("context").get<int>());
    return {parentBlock, context};
  }

  template <>
  api::SelectCompletion fromJson<api::SelectCompletion>(const nlohmann::json& j) {
    FullID parentBlock = j.at("parentBlock").get<std::vector<uint64_t>>();
    return {parentBlock, j.at("selected").get<std::string>()};
  }

  template <>
  api::RequestDiagnostics fromJson<api::RequestDiagnostics>(const nlohmann::json& j) {
    return {j.at("path").get<std::string>()};
  }

}  // namespace fluir::lsp
