#include "server.hpp"

#include <set>

#include <asio.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "lsp/database/database.hpp"

class FakeDatabase : public fluir::lsp::LanguageDatabase {
 public:
  struct SetFileContentsCall {
    std::filesystem::path path;
    std::string contents;
  };

  std::vector<SetFileContentsCall> setFileContentsCalls;
  std::vector<std::filesystem::path> invalidateCalls;

  void setFileContents(std::filesystem::path file, std::string contents) override {
    openedFiles.insert(file);
    setFileContentsCalls.push_back({std::move(file), std::move(contents)});
  }

  void invalidate(const std::filesystem::path& file) override { invalidateCalls.push_back(file); }

  std::span<const fluir::lsp::api::ModuleDiagnostic> diagnostics(const std::filesystem::path&) override { return {}; }

  std::optional<fluir::lsp::api::Symbol> symbolAt(const std::filesystem::path&, fluir::FullID) override {
    return std::nullopt;
  }

  std::optional<std::vector<fluir::lsp::api::TaggedSymbol>> allSymbols(const std::filesystem::path& file) override {
    if (openedFiles.contains(file)) {
      std::vector<fluir::lsp::api::TaggedSymbol> result;
      result.push_back(fluir::lsp::api::TaggedSymbol{
        .id = {1, 2, 3}, .symbol = {.name = "x", .detail = std::nullopt, .outType = "Int", .inType = std::nullopt}});
      return result;
    }
    return std::nullopt;
  }

  std::set<std::filesystem::path> openedFiles;
};

class ServerTest : public ::testing::Test {
 protected:
  using Channel = fluir::lsp::Channel;

  void SetUp() override {
    auto db = std::make_unique<FakeDatabase>();
    db_ = db.get();
    server_ = std::make_unique<fluir::lsp::Server>(requests_, responses_, std::move(db));
  }

  void processAll() {
    asio::co_spawn(ctx_, server_->processOne(), asio::detached);
    ctx_.run();
    ctx_.restart();
  }

  void runAll() {
    asio::co_spawn(ctx_, server_->run(), asio::detached);
    ctx_.run();
    ctx_.restart();
  }

  void send(nlohmann::json msg) { requests_.try_send(asio::error_code{}, std::move(msg)); }

  std::optional<nlohmann::json> tryReceive() {
    std::optional<nlohmann::json> result;
    responses_.try_receive([&](asio::error_code, nlohmann::json resp) { result = std::move(resp); });
    return result;
  }

  asio::io_context ctx_;
  Channel requests_{ctx_, 16};
  Channel responses_{ctx_, 16};
  FakeDatabase* db_ = nullptr;
  std::unique_ptr<fluir::lsp::Server> server_;
};

TEST_F(ServerTest, OpenDocCallsSetFileContents) {
  send({{"request", "OpenDoc"}, {"params", {{"path", "f.fl"}, {"content", "hello"}}}});
  processAll();

  ASSERT_EQ(db_->setFileContentsCalls.size(), 1);
  EXPECT_EQ(db_->setFileContentsCalls[0].path, "f.fl");
  EXPECT_EQ(db_->setFileContentsCalls[0].contents, "hello");

  auto response = tryReceive();
  ASSERT_TRUE(response.has_value());
  EXPECT_EQ((*response)["response"], "OpenDoc");
  EXPECT_TRUE((*response)["result"].is_object());
}

TEST_F(ServerTest, CloseDocCallsInvalidate) {
  send({{"request", "CloseDoc"}, {"params", {{"path", "f.fl"}}}});
  processAll();

  ASSERT_EQ(db_->invalidateCalls.size(), 1);
  EXPECT_EQ(db_->invalidateCalls[0], "f.fl");

  auto response = tryReceive();
  ASSERT_TRUE(response.has_value());
  EXPECT_EQ((*response)["response"], "CloseDoc");
  EXPECT_TRUE((*response)["result"].is_object());
}

TEST_F(ServerTest, ProcessMultipleRequests) {
  send({{"request", "OpenDoc"}, {"params", {{"path", "a.fl"}, {"content", "x"}}}});
  send({{"request", "CloseDoc"}, {"params", {{"path", "a.fl"}}}});

  asio::co_spawn(ctx_, server_->processOne(), asio::detached);
  asio::co_spawn(ctx_, server_->processOne(), asio::detached);
  ctx_.run();
  ctx_.restart();

  auto response1 = tryReceive();
  ASSERT_TRUE(response1.has_value());
  EXPECT_EQ((*response1)["response"], "OpenDoc");

  auto response2 = tryReceive();
  ASSERT_TRUE(response2.has_value());
  EXPECT_EQ((*response2)["response"], "CloseDoc");
}

TEST_F(ServerTest, RunProcessesRequestsThenStopsOnShutdown) {
  send({{"request", "OpenDoc"}, {"params", {{"path", "f.fl"}, {"content", "hello"}}}});
  send({{"request", "Shutdown"}, {"params", nlohmann::json::object()}});

  runAll();

  EXPECT_EQ(db_->setFileContentsCalls.size(), 1);

  auto r1 = tryReceive();
  ASSERT_TRUE(r1.has_value());
  EXPECT_EQ((*r1)["response"], "OpenDoc");

  auto r2 = tryReceive();
  ASSERT_TRUE(r2.has_value());
  EXPECT_EQ((*r2)["response"], "Shutdown");
}

TEST_F(ServerTest, RunStopsImmediatelyOnShutdown) {
  send({{"request", "Shutdown"}, {"params", nlohmann::json::object()}});

  runAll();

  auto response = tryReceive();
  ASSERT_TRUE(response.has_value());
  EXPECT_EQ((*response)["response"], "Shutdown");
}

TEST_F(ServerTest, ShutdownReturnsResponse) {
  send({{"request", "Shutdown"}, {"params", nlohmann::json::object()}});
  processAll();

  auto response = tryReceive();
  ASSERT_TRUE(response.has_value());
  EXPECT_EQ((*response)["response"], "Shutdown");
  EXPECT_EQ((*response)["result"], nlohmann::json::object());
}

TEST_F(ServerTest, InitReturnsVersion) {
  send({{"request", "Init"}, {"params", nlohmann::json::object()}});
  processAll();

  auto response = tryReceive();
  ASSERT_TRUE(response.has_value());
  EXPECT_EQ((*response)["response"], "Init");
  EXPECT_EQ((*response)["result"]["version"], "0.3.0");
}

TEST_F(ServerTest, SymbolsReturnsSymbols) {
  send({{"request", "OpenDoc"}, {"params", {{"path", "f.fl"}, {"content", "hello"}}}});
  processAll();
  tryReceive();  // discard OpenDoc response

  send({{"request", "Symbols"}, {"params", {{"path", "f.fl"}}}});
  processAll();

  auto response = tryReceive();
  ASSERT_TRUE(response.has_value());
  EXPECT_EQ((*response)["response"], "Symbols");
  ASSERT_TRUE((*response)["result"]["symbols"].is_array());
  EXPECT_EQ((*response)["result"]["symbols"].size(), 1);
  EXPECT_EQ((*response)["result"]["symbols"][0]["symbol"]["name"], "x");
}

TEST_F(ServerTest, SymbolsForUnknownFileReturnsEmptySymbols) {
  send({{"request", "Symbols"}, {"params", {{"path", "unknown.fl"}}}});
  processAll();

  auto response = tryReceive();
  ASSERT_TRUE(response.has_value());
  EXPECT_EQ((*response)["response"], "Symbols");
  ASSERT_TRUE((*response)["result"]["symbols"].is_array());
  EXPECT_EQ((*response)["result"]["symbols"].size(), 0);
}

TEST_F(ServerTest, UnknownRequestIsIgnored) {
  send({{"request", "Bogus"}, {"params", nlohmann::json::object()}});
  processAll();

  auto response = tryReceive();
  EXPECT_FALSE(response.has_value());
}
