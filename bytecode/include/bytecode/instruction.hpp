#ifndef FLUIR_BYTECODE_INSTRUCTION_HPP
#define FLUIR_BYTECODE_INSTRUCTION_HPP

#include <cstdint>

namespace fluir::code {
  // clang-format off
  #define FLUIR_CODE_INSTRUCTIONS(code)  \
  code(EXIT)                             \
  code(PUSH)                             \
  code(POP)                              \
  code(F64_ADD)                          \
  code(F64_SUB)                          \
  code(F64_MUL)                          \
  code(F64_DIV)                          \
  code(F64_NEG)                          \
  code(F64_AFF)                          \
  code(I64_ADD)                          \
  code(I64_SUB)                          \
  code(I64_MUL)                          \
  code(I64_DIV)                          \
  code(I64_NEG)                          \
  code(I64_AFF)                          \
  code(U64_ADD)                          \
  code(U64_SUB)                          \
  code(U64_MUL)                          \
  code(U64_DIV)                          \
  code(U64_AFF)                          \
  code(CAST_I2U)                         \
  code(CAST_U2I)                         \
  code(CAST_I2F)                         \
  code(CAST_U2F)                         \
  code(CAST_F2I)                         \
  code(CAST_F2U)                         \
  code(CAST_WIDTH)

  // clang-format on

  enum Instruction : std::uint8_t {
#define enumerate(inst) inst,
    FLUIR_CODE_INSTRUCTIONS(enumerate)
#undef enumerate
  };
}  // namespace fluir::code

#endif
