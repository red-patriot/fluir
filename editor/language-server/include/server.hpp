#ifndef FLUIR_LSP_SERVER_HPP
#define FLUIR_LSP_SERVER_HPP

#include <memory>

#include <asio.hpp>
#include <nlohmann/json.hpp>

#include "lsp/api/document.hpp"
#include "lsp/channel.hpp"
#include "lsp/database/database.hpp"

namespace fluir::lsp {
  /** Handles intelligence requests for Fluir */
  class Server {
   public:
    Server(Channel& requests, Channel& responses, std::unique_ptr<LanguageDatabase> db);

    /** Processes the next message */
    asio::awaitable<void> processOne();

   private:
    nlohmann::json dispatch(const nlohmann::json& msg);
    nlohmann::json openDoc(const api::OpenDocRequest& req);
    nlohmann::json closeDoc(const api::CloseDocRequest& req);

    std::unique_ptr<LanguageDatabase> db_; /**< The underlying database for intelligence */
    Channel& requests_;                    /**< The channel to receive requests */
    Channel& responses_;                   /**< The channel to send responses */
  };

}  // namespace fluir::lsp

#endif
