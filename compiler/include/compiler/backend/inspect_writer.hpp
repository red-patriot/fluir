#ifndef FLUIR_COMPILER_BACKEND_INSPECT_WRITER_HPP
#define FLUIR_COMPILER_BACKEND_INSPECT_WRITER_HPP

#include <fmt/format.h>

#include "compiler/backend/code_writer.hpp"
#include "compiler/utility/indent_formatter.hpp"

namespace fluir {
  class InspectWriter : public CodeWriter, private IndentFormatter<> {
   public:
    using CodeWriter::CodeWriter;

    void writeHeader(const code::Header&) override;
    void writeConstants(const be::ConstantsArray&) override;
    void writeChunk(const code::Chunk&) override;

   private:
    void writeConstant(const be::Constant&);
    void writeCode(const code::Bytes&);

    void emitInstruction(uint8_t instruction);
    void emitInstructionWithArg(uint8_t instruction, uint8_t arg);
    void emitLongArg(uint8_t arg0, uint8_t arg1, uint8_t arg2, uint8_t arg3);
  };
}  // namespace fluir

#endif
