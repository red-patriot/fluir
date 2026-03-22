#include "options.hpp"

#include <algorithm>
#include <iostream>

#include <argparse/argparse.hpp>

namespace fluir::lsp {
  std::optional<Options> parseArgs(const std::vector<std::string>& args) {
    argparse::ArgumentParser parser("Fluir language server");

    parser.add_argument("--port", "-p").help("The port number to listen on.").required().scan<'u', uint16_t>();
    parser.add_argument("-v").help("Enable debug logging.").flag();

    try {
      parser.parse_args(args);
    } catch (const std::runtime_error&) {
      std::cout << parser << '\n';
      return std::nullopt;
    }

    return Options{
      .port = parser.get<uint16_t>("--port"),
      .logLevel = parser.get<bool>("-v") ? LogLevel::DEBUG : LogLevel::NORMAL,
    };
  }

  std::optional<Options> parseArgs(int argc, const char** argv) {
    return parseArgs(std::vector<std::string>(argv, argv + argc));
  }
}  // namespace fluir::lsp
