module;

#include <cstdint>
#include "byte_code/instruction.hpp"

export module fluir.instruction;

namespace fluir::code {
  export enum Instruction : std::uint8_t {
#define enumerate(inst) inst,
    FLUIR_CODE_INSTRUCTIONS(enumerate)
#undef enumerate
  };
}  // namespace fluir::code
