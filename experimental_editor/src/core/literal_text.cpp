#include "editor/core/literal_text.hpp"

#include <array>
#include <charconv>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <type_traits>
#include <variant>

#include <fmt/format.h>

#include "compiler/frontend/text_parse.hpp"

namespace fluir::editor {
  using namespace ::fluir::literals_types;

  // I8 / U8 widen to int
  // so they print as numbers, BOOL prints true/false, and every other
  // arithmetic type goes straight through fmt.
  std::string renderLiteral(const et::Literal& value) {
    return std::visit(
      [](auto v) -> std::string {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, bool>) {
          return v ? "true" : "false";
        } else if constexpr (std::is_same_v<T, std::int8_t> || std::is_same_v<T, std::uint8_t>) {
          return fmt::format("{}", static_cast<int>(v));
        } else {
          return fmt::format("{}", v);
        }
      },
      value);
  }

  std::string_view literalTypeName(const et::Literal& value) {
    // Indexed in `et::Literal` alternative order.
    static constexpr std::array<std::string_view, std::variant_size_v<et::Literal>> kNames{
      "f64", "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "bool"};
    return kNames[value.index()];
  }

  bool isEditableLiteral(const et::Literal& value) {
    return std::visit(
      [](auto v) {
        using T = std::decay_t<decltype(v)>;
        return (std::integral<T> && !std::is_same_v<T, fluir::literals_types::BOOL>) ||
               std::is_same_v<T, fluir::literals_types::F64>;
      },
      value);
  }

  std::optional<et::Literal> tryParseLiteral(const et::Literal& like, std::string_view text) {
    return std::visit(
      [text](auto v) -> std::optional<et::Literal> {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::integral<T> && !std::is_same_v<T, bool>) {
          const auto parsed = fe::parseNumber<T>(text);
          return parsed ? std::optional<et::Literal>(et::Literal(std::in_place_type<T>, *parsed)) : std::nullopt;
        } else if constexpr (std::is_same_v<T, double>) {
          // Whole text must parse; inf/nan/overflow are rejected like integer range errors.
          double d{};
          const auto [end, ec] = std::from_chars(text.data(), text.data() + text.size(), d);
          if (ec != std::errc{} || end != text.data() + text.size() || !std::isfinite(d)) {
            return std::nullopt;
          }
          return et::Literal(std::in_place_type<T>, d);
        } else {
          return std::nullopt;
        }
      },
      like);
  }

}  // namespace fluir::editor
