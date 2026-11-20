#include "compiler/frontend/text_parse.hpp"

#include <charconv>
#include <cstdlib>
#include <limits>
#include <numbers>
#include <unordered_map>

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

  // TODO: make this better
  std::expected<double, NumberParseError> parseDouble(std::string_view text) {
    static const std::unordered_map<std::string_view, double> NAMED_CONSTANTS{
      {"pi", std::numbers::pi},
      {"e", std::numbers::e},
      {"ln(2)", std::numbers::ln2},
      {"ln(10)", std::numbers::ln10},
      {"sqrt(2)", std::numbers::sqrt2},
      {"sqrt(3)", std::numbers::sqrt3},
      {"inf", std::numeric_limits<double>::infinity()},
    };
    if (NAMED_CONSTANTS.contains(text)) {
      return NAMED_CONSTANTS.at(text);
    }

    double d = 0;
    auto result = std::from_chars(text.data(), text.data() + text.size(), d);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
      return std::unexpected(CANNOT_PARSE_NUMBER);
    }
    return d;
  }
}  // namespace fluir::fe
