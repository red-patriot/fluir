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
    spdlog::info("Server started");
    while (true) {
      auto msg = co_await requests_.async_receive(asio::use_awaitable);
      auto result = dispatch(msg);
      if (result) co_await responses_.async_send(asio::error_code{}, std::move(*result), asio::use_awaitable);
      if (msg.at("request").get<std::string>() == "Shutdown") {
        spdlog::info("Server shutting down");
        break;
      }
    }
  }

  std::optional<nlohmann::json> Server::dispatch(const nlohmann::json& msg) {
    auto name = msg.at("request").get<std::string>();
    spdlog::info("Received request: {}", name);

    try {
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
    } catch (const std::exception& e) {
      spdlog::error("Error handling '{}': {}", name, e.what());
    }
    return std::nullopt;
  }

  nlohmann::json Server::openDoc(const api::OpenDocRequest& req) {
    spdlog::debug("Opening document: {}", req.path);
    db_->setFileContents(req.path, req.content);
    return toJson(api::OpenDocResponse{});
  }

  nlohmann::json Server::closeDoc(const api::CloseDocRequest& req) {
    spdlog::debug("Closing document: {}", req.path);
    db_->invalidate(req.path);
    return toJson(api::CloseDocResponse{});
  }

  nlohmann::json Server::symbols(const api::SymbolRequest& req) {
    spdlog::debug("Symbols requested for: {}", req.path);
    auto result = db_->allSymbols(req.path);
    if (!result) {
      spdlog::debug("No symbols found for: {}", req.path);
      return toJson(api::Symbols{});
    }
    return toJson(api::Symbols{.symbols = std::move(*result)});
  }

}  // namespace fluir::lsp
