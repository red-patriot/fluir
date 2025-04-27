#ifndef FLUIR_BYTECODE_OP_CODE_HPP
#define FLUIR_BYTECODE_OP_CODE_HPP

#include <cstdint>

namespace fluir::code {
  enum OpCode : std::uint8_t {
    EXIT,
    PUSH_FP,
    POP,
    FP_ADD,
    FP_SUBTRACT,
    FP_MULTIPLY,
    FP_DIVIDE,
  };
}

#endif
