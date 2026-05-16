#ifndef FLUIR_VM_MACHINE_CALL_FRAME_HPP
#define FLUIR_VM_MACHINE_CALL_FRAME_HPP

#include <cstdint>

#include <bytecode/code_chunk.hpp>
#include <vm/code/value.hpp>

namespace fluir {
  struct CallFrame {
    code::Chunk const* chunk;
    std::uint8_t const* returnAddress;
    code::Value* basePtr;
    code::Value* stackEnd;
  };

  inline void setReturn(CallFrame& frame, code::Value val) { *frame.basePtr = std::move(val); }
  inline void pop(CallFrame& frame) { --frame.stackEnd; }
  inline void push(CallFrame& frame, code::Value value) {
    (*frame.stackEnd) = std::move(value);
    ++frame.stackEnd;
  }
  inline const code::Value& stackTop(const CallFrame& frame) { return *(frame.stackEnd - 1); }
}  // namespace fluir

#endif
