#ifndef FLUIR_BYTECODE_INSTRUCTION_HPP
#define FLUIR_BYTECODE_INSTRUCTION_HPP

// clang-format off
  #define FLUIR_CODE_INSTRUCTIONS(code)  \
  code(EXIT)                             \
  code(PUSH_FP)                          \
  code(POP)                              \
  code(FP_ADD)                           \
  code(FP_SUBTRACT)                      \
  code(FP_MULTIPLY)                      \
  code(FP_DIVIDE)                        \
  code(FP_NEGATE)                        \
  code(FP_AFFIRM)

// clang-format on

#endif
