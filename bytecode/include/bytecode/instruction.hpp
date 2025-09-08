#ifndef FLUIR_BYTECODE_INSTRUCTION_HPP
#define FLUIR_BYTECODE_INSTRUCTION_HPP

#include <cstdint>

namespace fluir::code {
  // clang-format off
  #define FLUIR_CODE_INSTRUCTIONS(code)  \
  code(EXIT)                             \
  code(PUSH_F64)                         \
  code(POP)                              \
  code(F64_ADD)                          \
  code(F64_SUB)                          \
  code(F64_MUL)                          \
  code(F64_DIV)                          \
  code(F64_NEG)                          \
  code(F64_AFF)

  // clang-format on

  enum Instruction : std::uint8_t {
#define enumerate(inst) inst,
    FLUIR_CODE_INSTRUCTIONS(enumerate)
#undef enumerate
  };
}  // namespace fluir::code

#endif
