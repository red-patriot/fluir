#ifndef FLUIR_COMPILER_FRONTEND_TEXT_PARSE_HPP
#define FLUIR_COMPILER_FRONTEND_TEXT_PARSE_HPP

#include <concepts>
#include <cstdint>
#include <expected>
#include <string>

namespace fluir::fe {
  enum class NumberParseError {
    SUCCESS = 0,
    CANNOT_PARSE_NUMBER = 1,
    RESULT_OUT_OF_BOUNDS,
  };

  /** Parses text as the given integer type */
  template <std::integral T>
  std::expected<T, NumberParseError> parseNumber(std::string_view text);
  std::expected<int64_t, NumberParseError> parseInteger(std::string_view text);
  std::expected<uint64_t, NumberParseError> parseUnsigned(std::string_view text);
  template <std::integral Out, std::integral In>
  std::expected<Out, NumberParseError> checkBounds(const In& number);

  template <std::integral T>
  inline std::expected<T, NumberParseError> parseNumber(std::string_view text) {
    if constexpr (std::is_signed_v<T>) {
      return parseInteger(text).and_then(checkBounds<T, int64_t>);
    } else {
      return parseUnsigned(text).and_then(checkBounds<T, uint64_t>);
    }
  }

  template <std::integral Out, std::integral In>
  inline std::expected<Out, NumberParseError> checkBounds(const In& number) {
    if (number < std::numeric_limits<Out>::min() || number > std::numeric_limits<Out>::max()) {
      return std::unexpected(NumberParseError::RESULT_OUT_OF_BOUNDS);
    }
    return static_cast<Out>(number);
  }

}  // namespace fluir::fe

#endif
