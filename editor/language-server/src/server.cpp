#include "server.hpp"

#include <format>

#include <asio/use_awaitable.hpp>
#include <spdlog/spdlog.h>

#include "bytecode/version.hpp"
#include "lsp/api/language.hpp"
#include "lsp/api/lifecycle.hpp"
#include "lsp/json_serialize.hpp"

namespace fluir::lsp {

  Server::Server(Channel& requests, Channel& responses, std::unique_ptr<LanguageDatabase> db) :
    db_{std::move(db)}, requests_{requests}, responses_{responses} { }

  asio::awaitable<void> Server::processOne() {
    auto msg = co_await requests_.async_receive(asio::use_awaitable);
    auto result = dispatch(msg);
    if (result) co_await responses_.async_send(asio::error_code{}, std::move(*result), asio::use_awaitable);
  }

  asio::awaitable<void> Server::run() {
    while (true) {
      auto msg = co_await requests_.async_receive(asio::use_awaitable);
      auto result = dispatch(msg);
      if (result) co_await responses_.async_send(asio::error_code{}, std::move(*result), asio::use_awaitable);
      if (msg.at("request").get<std::string>() == "Shutdown") break;
    }
  }

  std::optional<nlohmann::json> Server::dispatch(const nlohmann::json& msg) {
    auto name = msg.at("request").get<std::string>();
    spdlog::info("Received request: {}", name);

    if (name == "Init") {
      auto version = std::format("{}.{}.{}", CURRENT_VERSION.major, CURRENT_VERSION.minor, CURRENT_VERSION.patch);
      return nlohmann::json{{"response", name}, {"result", toJson(api::InitResponse{.version = version})}};
    }

    if (name == "OpenDoc") {
      auto req = fromJson<api::OpenDocRequest>(msg.at("params"));
      auto resp = openDoc(req);
      return nlohmann::json{{"response", name}, {"result", resp}};
    }

    if (name == "CloseDoc") {
      auto req = fromJson<api::CloseDocRequest>(msg.at("params"));
      auto resp = closeDoc(req);
      return nlohmann::json{{"response", name}, {"result", resp}};
    }

    if (name == "Symbols") {
      auto req = fromJson<api::SymbolRequest>(msg.at("params"));
      auto resp = symbols(req);
      return nlohmann::json{{"response", name}, {"result", resp}};
    }

    if (name == "Shutdown") {
      return nlohmann::json{{"response", name}, {"result", toJson(api::ShutdownResponse{})}};
    }

    spdlog::error("Unrecognized command '{}'", name);
    return std::nullopt;
  }

  nlohmann::json Server::openDoc(const api::OpenDocRequest& req) {
    db_->setFileContents(req.path, req.content);
    return toJson(api::OpenDocResponse{});
  }

  nlohmann::json Server::closeDoc(const api::CloseDocRequest& req) {
    db_->invalidate(req.path);
    return toJson(api::CloseDocResponse{});
  }

  nlohmann::json Server::symbols(const api::SymbolRequest& req) {
    auto result = db_->allSymbols(req.path);
    if (!result) {
      return toJson(api::Symbols{});
    }
    return toJson(api::Symbols{.symbols = std::move(*result)});
  }

}  // namespace fluir::lsp
