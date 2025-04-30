#ifndef FLUIR_BYTECODE_INSTRUCTION_HPP
#define FLUIR_BYTECODE_INSTRUCTION_HPP

#include <cstdint>

namespace fluir::code {
  enum Instruction : std::uint8_t {
    EXIT,
    PUSH_FP,
    POP,
    FP_ADD,
    FP_SUBTRACT,
    FP_MULTIPLY,
    FP_DIVIDE,
    FP_NEGATE,
    FP_AFFIRM,
  };
}

#endif
