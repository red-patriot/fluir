#ifndef FLUIR_LSP_CHANNEL_HPP
#define FLUIR_LSP_CHANNEL_HPP

#include <asio/error_code.hpp>
#include <asio/experimental/concurrent_channel.hpp>
#include <nlohmann/json.hpp>

namespace fluir::lsp {
  using Channel = asio::experimental::concurrent_channel<void(asio::error_code, nlohmann::json)>;

}

#endif
