#ifndef FLUIR_VM_CODE_BYTE_CODE_HPP
#define FLUIR_VM_CODE_BYTE_CODE_HPP

#include <vector>

#include <bytecode/code_chunk.hpp>
#include <bytecode/header.hpp>
#include <bytecode/instruction.hpp>

#include "vm/code/value.hpp"

namespace fluir::code {

  struct ByteCode {
    Header header{};
    std::vector<Value> constants{};
    std::vector<Chunk> chunks{};
  };
}  // namespace fluir::code

#endif
