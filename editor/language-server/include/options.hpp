#ifndef FLUIR_LSP_OPTIONS_HPP
#define FLUIR_LSP_OPTIONS_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace fluir::lsp {
  enum class LogLevel { NORMAL, DEBUG };

  struct Options {
    uint16_t port;
    LogLevel logLevel{LogLevel::NORMAL};

    friend bool operator==(const Options&, const Options&) = default;
  };

  std::optional<Options> parseArgs(int argc, const char** argv);
  std::optional<Options> parseArgs(const std::vector<std::string>& args);

}  // namespace fluir::lsp

#endif
