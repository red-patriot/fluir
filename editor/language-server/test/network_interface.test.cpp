#include "lsp/network_interface.hpp"

#include <deque>
#include <string>

#include <asio.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "lsp/channel.hpp"

namespace {

  class FakeStream {
   public:
    explicit FakeStream(asio::io_context& ctx) : ctx_(ctx) { }

    void enqueue(std::string chunk) { chunks_.push_back(std::move(chunk)); }

    void close() { closed_ = true; }

    template <typename MutableBufferSequence>
    asio::awaitable<std::size_t> async_read_some(const MutableBufferSequence& buffers) {
      // Yield to let other coroutines run
      co_await asio::post(ctx_, asio::use_awaitable);

      if (chunks_.empty()) {
        if (closed_) {
          throw asio::system_error(asio::error::eof);
        }
        // Should not happen in tests — means test setup is wrong
        throw std::runtime_error("FakeStream: no data and not closed");
      }

      auto& front = chunks_.front();
      auto n = asio::buffer_copy(buffers, asio::buffer(front));
      if (n >= front.size()) {
        chunks_.pop_front();
      } else {
        front.erase(0, n);
      }
      co_return n;
    }

   private:
    asio::io_context& ctx_;
    std::deque<std::string> chunks_;
    bool closed_ = false;
  };

  std::string frameMessage(const nlohmann::json& j) {
    std::string body = j.dump();
    std::ostringstream header;
    header << "LENGTH: " << std::hex << body.size() << "\r\n";
    return header.str() + body;
  }

  class NetworkInterfaceTest : public ::testing::Test {
   protected:
    using Channel = fluir::lsp::Channel;

    void run() {
      asio::co_spawn(
        ctx_,
        [this]() -> asio::awaitable<void> {
          fluir::lsp::NetworkInterface<FakeStream> iface(stream_, output_);
          co_await iface.run();
        },
        asio::detached);
      ctx_.run();
      ctx_.restart();
    }

    std::vector<nlohmann::json> collectAll() {
      std::vector<nlohmann::json> results;
      while (true) {
        std::optional<nlohmann::json> msg;
        output_.try_receive([&](asio::error_code, nlohmann::json j) { msg = std::move(j); });
        if (!msg) break;
        results.push_back(std::move(*msg));
      }
      return results;
    }

    asio::io_context ctx_;
    Channel output_{ctx_, 16};
    FakeStream stream_{ctx_};
  };

  TEST_F(NetworkInterfaceTest, SingleValidMessage) {
    nlohmann::json msg = {{"request", "Init"}, {"params", nlohmann::json::object()}};
    stream_.enqueue(frameMessage(msg));
    stream_.close();
    run();

    auto results = collectAll();
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]["request"], "Init");
  }

  TEST_F(NetworkInterfaceTest, StreamClosesCleanly) {
    nlohmann::json msg = {{"request", "Init"}, {"params", nlohmann::json::object()}};
    stream_.enqueue(frameMessage(msg));
    stream_.close();
    run();

    auto results = collectAll();
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]["request"], "Init");
  }

  TEST_F(NetworkInterfaceTest, EmptyStreamClosesImmediately) {
    stream_.close();
    run();

    auto results = collectAll();
    EXPECT_TRUE(results.empty());
  }

  TEST_F(NetworkInterfaceTest, MultipleMessages) {
    nlohmann::json msg1 = {{"request", "Init"}, {"params", nlohmann::json::object()}};
    nlohmann::json msg2 = {{"request", "Shutdown"}, {"params", nlohmann::json::object()}};
    stream_.enqueue(frameMessage(msg1));
    stream_.enqueue(frameMessage(msg2));
    stream_.close();
    run();

    auto results = collectAll();
    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0]["request"], "Init");
    EXPECT_EQ(results[1]["request"], "Shutdown");
  }

  TEST_F(NetworkInterfaceTest, InvalidJsonIsSkipped) {
    // Broken JSON with valid header
    std::string broken = "{broken";
    std::ostringstream header;
    header << "LENGTH: " << std::hex << broken.size() << "\r\n";
    stream_.enqueue(header.str() + broken);

    // Followed by a valid message
    nlohmann::json valid = {{"request", "Init"}, {"params", nlohmann::json::object()}};
    stream_.enqueue(frameMessage(valid));
    stream_.close();
    run();

    auto results = collectAll();
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]["request"], "Init");
  }

  TEST_F(NetworkInterfaceTest, MalformedHeaderIsSkipped) {
    stream_.enqueue("BOGUS\r\n");

    nlohmann::json valid = {{"request", "Init"}, {"params", nlohmann::json::object()}};
    stream_.enqueue(frameMessage(valid));
    stream_.close();
    run();

    auto results = collectAll();
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]["request"], "Init");
  }

  TEST_F(NetworkInterfaceTest, PartialReads) {
    nlohmann::json msg = {{"request", "Init"}, {"params", nlohmann::json::object()}};
    std::string full = frameMessage(msg);

    // Split into small chunks of 3 bytes each
    for (std::size_t i = 0; i < full.size(); i += 3) {
      stream_.enqueue(full.substr(i, 3));
    }
    stream_.close();
    run();

    auto results = collectAll();
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]["request"], "Init");
  }

  TEST_F(NetworkInterfaceTest, LargeMessage) {
    nlohmann::json msg;
    msg["request"] = "OpenDoc";
    // ~10KB of content
    msg["params"] = {{"path", "f.fl"}, {"content", std::string(10000, 'x')}};
    stream_.enqueue(frameMessage(msg));
    stream_.close();
    run();

    auto results = collectAll();
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]["request"], "OpenDoc");
    EXPECT_EQ(results[0]["params"]["content"].get<std::string>().size(), 10000);
  }

}  // namespace
