#ifndef FLUIR_BYTECODE_PRIMITIVES_HPP
#define FLUIR_BYTECODE_PRIMITIVES_HPP

#include <cstdint>

namespace fluir::code {
  // clang-format off
#define FLUIR_CODE_PRIMITIVE_TYPES(code)  \
code(F64, double)                   \
code(I8, std::int8_t)               \
code(I16, std::int16_t)             \
code(I32, std::int32_t)             \
code(I64, std::int64_t)             \
code(U8, std::uint8_t)              \
code(U16, std::uint16_t)            \
code(U32, std::uint32_t)            \
code(U64, std::uint64_t)
  // clang-format on

  enum NumericWidth : std::uint8_t {
    WIDTH_8 = 0b0001,
    WIDTH_16 = 0b0010,
    WIDTH_32 = 0b0100,
    WIDTH_64 = 0b1000,
  };

  enum NumericCategory {
    SIGNED = 0b00010000,
    UNSIGNED = 0b00000000,
    FLOAT = 0b10000000,
  };

  constexpr uint8_t operator|(NumericWidth width, NumericCategory category) {
    return static_cast<uint8_t>(category) | static_cast<uint8_t>(width);
  }

  constexpr uint8_t operator|(NumericCategory category, NumericWidth width) { return width | category; }

  enum class PrimitiveType {
    F64 = FLOAT | WIDTH_64,
    I8 = SIGNED | WIDTH_8,
    I16 = SIGNED | WIDTH_16,
    I32 = SIGNED | WIDTH_32,
    I64 = SIGNED | WIDTH_64,
    U8 = UNSIGNED | WIDTH_8,
    U16 = UNSIGNED | WIDTH_16,
    U32 = UNSIGNED | WIDTH_32,
    U64 = UNSIGNED | WIDTH_64,
  };

#define FLUIR_ALIAS(Type, Concrete) using Type = Concrete;
  FLUIR_CODE_PRIMITIVE_TYPES(FLUIR_ALIAS)
#undef FLUIR_ALIAS
}  // namespace fluir::code
#endif
