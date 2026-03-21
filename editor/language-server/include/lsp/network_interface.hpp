#ifndef FLUIR_LSP_NETWORK_INTERFACE_HPP
#define FLUIR_LSP_NETWORK_INTERFACE_HPP

#include <cstddef>
#include <sstream>
#include <string>

#include <asio.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "lsp/channel.hpp"

namespace fluir::lsp {

  template <typename T>
  concept AsyncReadStream = requires(T& stream, asio::mutable_buffer buf) { stream.async_read_some(buf); };

  template <typename T>
  concept AsyncWriteStream =
    requires(T& stream, asio::const_buffer buf, asio::use_awaitable_t<> token) { stream.async_write_some(buf, token); };

  /**
   * Bidirectional wire-protocol interface.
   * Reads incoming LENGTH-framed JSON from the stream into the receive channel,
   * and writes outgoing JSON from the send channel onto the stream.
   */
  template <typename Stream>
    requires AsyncReadStream<Stream> && AsyncWriteStream<Stream>
  class NetworkInterface {
   public:
    NetworkInterface(Stream& stream, Channel& incoming, Channel& send) :
      stream_(stream), incoming_(incoming), send_(send) { }

    asio::awaitable<void> run() {
      using namespace asio::experimental::awaitable_operators;
      co_await (receiveLoop() || sendLoop());
    }

   private:
    asio::awaitable<void> receiveLoop() {
      try {
        while (true) {
          auto line = co_await readLine();

          if (line.rfind("LENGTH: ", 0) != 0) {
            continue;
          }

          std::string hexStr = line.substr(8);
          auto bodyLength = tryParseLength(hexStr);
          if (!bodyLength) {
            continue;
          }

          auto body = co_await readExactly(*bodyLength);

          auto msg = tryParse(body);
          if (!msg) {
            continue;
          }

          co_await incoming_.async_send(asio::error_code{}, std::move(*msg), asio::use_awaitable);
        }
      } catch (const asio::system_error& e) {
        if (e.code() == asio::error::eof) {
          co_return;
        }
        throw;
      }
    }

    asio::awaitable<void> sendLoop() {
      try {
        while (true) {
          auto msg = co_await send_.async_receive(asio::use_awaitable);
          std::string body = msg.dump();
          std::ostringstream header;
          header << "LENGTH: " << std::hex << body.size() << "\r\n";
          std::string frame = header.str() + body;
          co_await asio::async_write(stream_, asio::buffer(frame), asio::use_awaitable);
        }
      } catch (const asio::system_error&) {
        co_return;
      }
    }

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

    static std::optional<size_t> tryParseLength(const std::string& chars) {
      try {
        return std::stoull(chars, nullptr, 16);
      } catch (const std::exception& e) {
        spdlog::error("Length could not be parsed. Got '{}', error: {}", chars, e.what());
      }
      return std::nullopt;
    }

    static std::optional<nlohmann::json> tryParse(std::string_view chars) {
      try {
        auto msg = nlohmann::json::parse(chars);
        return msg;
      } catch (const std::exception& e) {
        spdlog::error("Failed to parse JSON msg: {}", e.what());
      } catch (...) {
        spdlog::error("Unknown error occurred when parsing JSON msg");
      }
      return std::nullopt;
    }

    asio::awaitable<void> fillBuffer() {
      char tmp[4096];
      auto n = co_await stream_.async_read_some(asio::buffer(tmp, sizeof(tmp)));
      buffer_.append(tmp, n);
    }

    Stream& stream_;
    Channel& incoming_;
    Channel& send_;
    std::string buffer_;
  };

}  // namespace fluir::lsp

#endif
