#ifndef FLUIR_COMPILER_BACKEND_CODE_WRITER_HPP
#define FLUIR_COMPILER_BACKEND_CODE_WRITER_HPP

#include <ostream>

#include "bytecode/byte_code.hpp"

namespace fluir {
  class CodeWriter {
   public:
    void write(const code::ByteCode& code, std::ostream& destination);

   private:
    virtual void writeHeader(const code::Header&, std::ostream&) = 0;
    virtual void writeChunk(const code::Chunk&, std::ostream&) = 0;
  };
}  // namespace fluir

#endif
