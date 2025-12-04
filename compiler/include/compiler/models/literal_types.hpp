#ifndef FLUIR_COMPILER_MODELS_LITERAL_TYPES_HPP
#define FLUIR_COMPILER_MODELS_LITERAL_TYPES_HPP

#include <cstdint>
#include <string>
#include <variant>

namespace fluir::literals_types {
  static_assert(sizeof(double) == 8, "Fluir only supports platforms where a double is 64-bits");

  using F64 = double;
  using I8 = int8_t;
  using I16 = int16_t;
  using I32 = int32_t;
  using I64 = int64_t;
  using U8 = uint8_t;
  using U16 = uint16_t;
  using U32 = uint32_t;
  using U64 = uint64_t;

  using Literal = std::variant<F64, I8, I16, I32, I64, U8, U16, U32, U64>;  // TODO: Support other literal types

}  // namespace fluir::literals_types

#endif
