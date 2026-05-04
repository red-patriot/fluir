#include "compiler/backend/code_writer.hpp"

#include <iostream>

namespace fluir {
  CodeWriter::CodeWriter(std::ostream& os) : os_(os) { }

  void CodeWriter::write(const code::ByteCode& code, std::ostream&) {
    writeHeader(code.header);
    writeConstants(code.constants);
    for (const auto& chunk : code.chunks) {
      writeChunk(chunk);
    }
  }
}  // namespace fluir
