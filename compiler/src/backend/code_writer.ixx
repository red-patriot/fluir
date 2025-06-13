module;

#include <ostream>

export module fluir.backend.code_writer;
export import fluir.byte_code;

namespace fluir {
  export class CodeWriter {
   public:
    void write(const code::ByteCode& code, std::ostream& destination);

   private:
    virtual void writeHeader(const code::Header&, std::ostream&) = 0;
    virtual void writeChunk(const code::Chunk&, std::ostream&) = 0;
  };
}  // namespace fluir
