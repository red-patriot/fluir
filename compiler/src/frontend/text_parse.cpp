#include "compiler/frontend/text_parse.hpp"

namespace fluir::fe {
  using enum NumberParseError;

  std::expected<int64_t, NumberParseError> parseInteger(std::string_view text) {
    if (text.empty()) {
      return std::unexpected(CANNOT_PARSE_NUMBER);
    }
    // Do all the work sign flipped since abs(INT_MIN) > abs(INT_MAX), it reduces
    // edge cases in the intermediate steps of the calculations
    int64_t sign = -1;

    switch (text[0]) {
      case '-':
        sign = 1;
        [[fallthrough]];
      case '+':
        text = text.substr(1);
        break;
      default:
        break;
    }

    int64_t result = 0;
    for (const char c : text) {
      if (c >= '0' && c <= '9') {
        const int64_t digit = c - '0';
        if (const int64_t limit = (INT64_MIN + digit) / 10; result < limit) {
          return std::unexpected(RESULT_OUT_OF_BOUNDS);
        }
        result *= 10;
        result -= digit;
      } else {
        return std::unexpected(CANNOT_PARSE_NUMBER);
      }
    }

    if (result == INT64_MIN && sign < 0) {
      // This edge case won't be caught above, so we need to manage it here
      return std::unexpected(RESULT_OUT_OF_BOUNDS);
    }

    return result * sign;
  }

  std::expected<uint64_t, NumberParseError> parseUnsigned(std::string_view text) {
    if (text.empty()) {
      return std::unexpected(CANNOT_PARSE_NUMBER);
    }

    switch (text[0]) {
      case '+':
        text = text.substr(1);
        break;
      default:
        break;
    }

    uint64_t result = 0;
    for (const char c : text) {
      if (c >= '0' && c <= '9') {
        const uint64_t digit = c - '0';
        if (const uint64_t limit = (UINT64_MAX - digit) / 10; result > limit) {
          return std::unexpected(RESULT_OUT_OF_BOUNDS);
        }
        result *= 10;
        result += c - '0';
      } else {
        return std::unexpected(CANNOT_PARSE_NUMBER);
      }
    }

    return result;
  }
}  // namespace fluir::fe
