#ifndef FLUIR_VM_MACHINE_CALL_FRAME_HPP
#define FLUIR_VM_MACHINE_CALL_FRAME_HPP

#include <cstdint>

#include <bytecode/code_chunk.hpp>

namespace fluir {
  struct CallFrame {
    code::Chunk const* chunk;
    std::uint8_t const* returnAddress;
    code::Value* basePtr;
    code::Value* stackEnd;
  };

}  // namespace fluir

#endif
