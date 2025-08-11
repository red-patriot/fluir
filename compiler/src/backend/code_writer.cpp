#include "compiler/backend/code_writer.hpp"

namespace fluir {
  void CodeWriter::write(const code::ByteCode& code, std::ostream& destination) {
    writeHeader(code.header, destination);
    for (const auto& chunk : code.chunks) {
      writeChunk(chunk, destination);
    }
  }
}  // namespace fluir
