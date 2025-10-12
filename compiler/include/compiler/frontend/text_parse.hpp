#ifndef FLUIR_COMPILER_FRONTEND_TEXT_PARSE_HPP
#define FLUIR_COMPILER_FRONTEND_TEXT_PARSE_HPP

#include <concepts>
#include <cstdint>
#include <expected>
#include <string>

namespace fluir::fe {
  /** Parses text as the given integer type */
  template <std::integral T>
  std::expected<T, std::string> parseNumber(std::string_view text);
  std::expected<int64_t, std::string> parseInteger(std::string_view text);
  std::expected<uint64_t, std::string> parseUnsigned(std::string_view text);
  template <std::integral T>
  std::expected<T, std::string> checkBounds(const T& number);

  template <std::integral T>
  inline std::expected<T, std::string> parseNumber(std::string_view text) {
    if constexpr (std::is_signed_v<T>) {
      return parseInteger(text).and_then(checkBounds<T>);
    } else {
      return parseUnsigned(text).and_then(checkBounds<T>);
    }
  }

  template <std::integral T>
  inline std::expected<T, std::string> checkBounds(const T& number) {
    return number;
  }

}  // namespace fluir::fe

#endif
