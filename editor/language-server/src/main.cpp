#include <memory>

#include <asio.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <compiler/utility/options.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "lsp/channel.hpp"
#include "lsp/database/in_memory_db.hpp"
#include "lsp/network_interface.hpp"
#include "options.hpp"
#include "server.hpp"

namespace {

  using fluir::lsp::Channel;
  using fluir::lsp::InMemoryDB;
  using fluir::lsp::NetworkInterface;
  using fluir::lsp::Server;

  using namespace asio::experimental::awaitable_operators;

  using AwaitableSocket = asio::use_awaitable_t<>::as_default_on_t<asio::ip::tcp::socket>;

  asio::awaitable<void> session(AwaitableSocket socket, asio::io_context& ctx) {
    Channel requests(ctx, 16);
    Channel responses(ctx, 16);

    auto db = std::make_unique<InMemoryDB>(fluir::CompilerOptions{});
    Server server(requests, responses, std::move(db));
    NetworkInterface network(socket, requests, responses);

    co_await (server.run() || network.run());
  }

}  // namespace

int main(int argc, const char** argv) {
  auto options = fluir::lsp::parseArgs(argc, argv);
  if (!options) {
    return 1;
  }

  auto logger = spdlog::stdout_color_mt("fluir-lsp");
  logger->set_level(options->logLevel == fluir::lsp::LogLevel::DEBUG ? spdlog::level::debug : spdlog::level::info);
  spdlog::set_default_logger(logger);

  spdlog::info("Starting on port {}", options->port);

  asio::io_context ctx;
  auto endpoint = asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), options->port);
  asio::ip::tcp::acceptor acceptor(ctx, endpoint);

  asio::co_spawn(
    ctx,
    [&acceptor, &ctx]() -> asio::awaitable<void> {
      auto raw_socket = co_await acceptor.async_accept(asio::use_awaitable);
      spdlog::info("Client connected");
      AwaitableSocket socket(std::move(raw_socket));
      co_await session(std::move(socket), ctx);
    },
    asio::detached);

  ctx.run();
  spdlog::info("Server stopped");
  return 0;
}
