#include "server.hpp"

#include <asio/use_awaitable.hpp>

#include "lsp/json_serialize.hpp"

namespace fluir::lsp {

  Server::Server(Channel& requests, Channel& responses, std::unique_ptr<LanguageDatabase> db) :
    db_{std::move(db)}, requests_{requests}, responses_{responses} { }

  asio::awaitable<void> Server::processOne() {
    auto msg = co_await requests_.async_receive(asio::use_awaitable);
    auto result = dispatch(msg);
    co_await responses_.async_send(asio::error_code{}, std::move(result), asio::use_awaitable);
  }

  nlohmann::json Server::dispatch(const nlohmann::json& msg) {
    auto name = msg.at("request").get<std::string>();

    if (name == "OpenDoc") {
      auto req = fromJson<api::OpenDocRequest>(msg.at("params"));
      auto resp = openDoc(req);
      return {{"response", name}, {"result", resp}};
    }

    if (name == "CloseDoc") {
      auto req = fromJson<api::CloseDocRequest>(msg.at("params"));
      auto resp = closeDoc(req);
      return {{"response", name}, {"result", resp}};
    }

    return {{"error", "unknown request"}};
  }

  nlohmann::json Server::openDoc(const api::OpenDocRequest& req) {
    db_->setFileContents(req.path, req.content);
    return toJson(api::OpenDocResponse{});
  }

  nlohmann::json Server::closeDoc(const api::CloseDocRequest& req) {
    db_->invalidate(req.path);
    return toJson(api::CloseDocResponse{});
  }

}  // namespace fluir::lsp
