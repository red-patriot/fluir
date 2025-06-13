module;

#include <ostream>

module fluir.backend.code_writer;

namespace fluir {
  void CodeWriter::write(const code::ByteCode& code, std::ostream& destination) {
    writeHeader(code.header, destination);
    for (const auto& chunk : code.chunks) {
      writeChunk(chunk, destination);
    }
  }
}  // namespace fluir
