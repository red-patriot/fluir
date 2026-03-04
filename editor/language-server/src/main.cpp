#include <iostream>

#include <asio.hpp>

int main() {
  asio::io_context ctx;

  asio::post(ctx, [] { std::cout << "fluir language server: Asio initialized successfully\n"; });

  ctx.run();
  return 0;
}
