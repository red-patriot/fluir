#include "vm/debug.hpp"

#include <iostream>

#include "vm/machine/vm.hpp"

namespace fluir::debug {
  namespace {
#define STRINGIFY(i) #i
#define FLUIR_INSTRUCTION_TO_STR(inst) STRINGIFY(inst),
    std::array instructionNames{FLUIR_CODE_INSTRUCTIONS(FLUIR_INSTRUCTION_TO_STR)};
#undef FLUIR_INSTRUCTION_TO_STR
#undef STRINGIFY
  }  // namespace

  void printStack(std::span<const code::Value> stack) {
    std::cout << "\t[DEBUG]\t";
    for (const auto& value : stack) {
      std::cout << "[ " << value << " ] ";
    }
    std::cout << '\n';
  }

  void printInstruction(std::uint8_t inst) {
    std::cout << "\t[DEBUG]\t";
    if (inst >= instructionNames.size()) {
      std::cout << inst << '\n';
      return;
    }
    std::cout << instructionNames[inst] << '\n';
  }

}  // namespace fluir::debug
