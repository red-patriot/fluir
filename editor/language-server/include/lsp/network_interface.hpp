#ifndef FLUIR_LSP_NETWORK_INTERFACE_HPP
#define FLUIR_LSP_NETWORK_INTERFACE_HPP

#include <concepts>
#include <cstddef>
#include <sstream>
#include <string>

#include <asio.hpp>
#include <nlohmann/json.hpp>

#include "lsp/channel.hpp"

namespace fluir::lsp {

  template <typename T>
  concept AsyncReadStream = requires(T& stream, asio::mutable_buffer buf) { stream.async_read_some(buf); };

  /**
   * Reads the wire protocol (LENGTH: <hex>\r\n + JSON body) from a stream
   * and feeds parsed JSON into a Channel.
   */
  template <AsyncReadStream Stream>
  class NetworkInterface {
   public:
    NetworkInterface(Stream& stream, Channel& output) : stream_(stream), output_(output) { }

    asio::awaitable<void> run() {
      try {
        while (true) {
          auto line = co_await readLine();

          // Validate header format: "LENGTH: <hex>\r\n"
          // readLine strips \r\n, so we expect "LENGTH: <hex>"
          if (line.rfind("LENGTH: ", 0) != 0) {
            // TODO: log malformed header
            continue;
          }

          std::string hexStr = line.substr(8);
          std::size_t bodyLength = 0;
          try {
            bodyLength = std::stoull(hexStr, nullptr, 16);
          } catch (...) {
            // TODO: log invalid hex in header
            continue;
          }

          auto body = co_await readExactly(bodyLength);

          nlohmann::json msg;
          try {
            msg = nlohmann::json::parse(body);
          } catch (...) {
            // TODO: log invalid JSON
            continue;
          }

          co_await output_.async_send(asio::error_code{}, std::move(msg), asio::use_awaitable);
        }
      } catch (const asio::system_error& e) {
        if (e.code() == asio::error::eof) {
          co_return;
        }
        throw;
      }
    }

   private:
    asio::awaitable<std::string> readLine() {
      while (true) {
        auto pos = buffer_.find("\r\n");
        if (pos != std::string::npos) {
          std::string line = buffer_.substr(0, pos);
          buffer_.erase(0, pos + 2);
          co_return line;
        }
        co_await fillBuffer();
      }
    }

    asio::awaitable<std::string> readExactly(std::size_t n) {
      while (buffer_.size() < n) {
        co_await fillBuffer();
      }
      std::string result = buffer_.substr(0, n);
      buffer_.erase(0, n);
      co_return result;
    }

    asio::awaitable<void> fillBuffer() {
      char tmp[4096];
      auto n = co_await stream_.async_read_some(asio::buffer(tmp, sizeof(tmp)));
      buffer_.append(tmp, n);
    }

    Stream& stream_;
    Channel& output_;
    std::string buffer_;
  };

}  // namespace fluir::lsp

#endif
