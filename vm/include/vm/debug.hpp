#ifndef FLUIR_VM_DEBUG_HPP
#define FLUIR_VM_DEBUG_HPP

#include <cstdint>
#include <span>

#include "bytecode/value.hpp"

namespace fluir::debug {
  void printStack(std::span<const code::Value> stack);
  void printInstruction(std::uint8_t inst);
}  // namespace fluir::debug

#endif
