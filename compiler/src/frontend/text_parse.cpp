#include "compiler/frontend/text_parse.hpp"

namespace fluir::fe {
  std::expected<int64_t, std::string> parseInteger(std::string_view text) {
    int64_t result = 0;
    int64_t sign = 1;

    switch (text[0]) {
      case '-':
        sign = -1;
        [[fallthrough]];
      case '+':
        text = text.substr(1);
        break;
      default:
        break;
    }

    for (const char c : text) {
      if (c >= '0' && c <= '9') {
        result *= 10;
        result += c - '0';
      } else {
        return std::unexpected("Could not parse");
      }
    }

    return result * sign;
  }

  std::expected<uint64_t, std::string> parseUnsigned(std::string_view text) {
    uint64_t result = 0;

    switch (text[0]) {
      case '+':
        text = text.substr(1);
        break;
      default:
        break;
    }

    for (const char c : text) {
      if (c >= '0' && c <= '9') {
        result *= 10;
        result += c - '0';
      } else {
        return std::unexpected("Could not parse");
      }
    }

    return result;
  }
}  // namespace fluir::fe
