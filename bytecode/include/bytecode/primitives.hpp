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

#define enumerate(Type, ConcreteType) Type,
  enum class PrimitiveType { FLUIR_CODE_PRIMITIVE_TYPES(enumerate) };
#undef enumerate
}  // namespace fluir::code
#endif
