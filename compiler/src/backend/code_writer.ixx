module;

#include <ostream>

#include "bytecode/byte_code.hpp"

export module fluir.backend.code_writer;

namespace fluir {
  export class CodeWriter {
   public:
    void write(const code::ByteCode& code, std::ostream& destination);

   private:
    virtual void writeHeader(const code::Header&, std::ostream&) = 0;
    virtual void writeChunk(const code::Chunk&, std::ostream&) = 0;
  };
}  // namespace fluir
