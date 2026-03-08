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

}  // namespace fluir::lsp
