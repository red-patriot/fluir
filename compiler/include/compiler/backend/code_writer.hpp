#ifndef FLUIR_COMPILER_BACKEND_CODE_WRITER_HPP
#define FLUIR_COMPILER_BACKEND_CODE_WRITER_HPP

#include <ostream>

#include "bytecode/byte_code.hpp"
#include "compiler/backend/constant.hpp"

namespace fluir {
  class CodeWriter {
   public:
    explicit CodeWriter(std::ostream&);
    virtual ~CodeWriter() = default;

    void write(const code::ByteCode& code, std::ostream& destination);

    virtual void writeHeader(const code::Header&) = 0;
    virtual void writeConstants(const std::vector<code::Value>&) = 0;
    virtual void writeConstants(const be::ConstantsArray&) = 0;
    virtual void writeChunk(const code::Chunk&) = 0;

   protected:
    std::ostream& os_;
  };
}  // namespace fluir

#endif
