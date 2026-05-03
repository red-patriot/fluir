#ifndef FLUIR_COMPILER_BACKEND_INSPECT_WRITER_HPP
#define FLUIR_COMPILER_BACKEND_INSPECT_WRITER_HPP

#include <fmt/format.h>

#include "compiler/backend/code_writer.hpp"
#include "compiler/utility/indent_formatter.hpp"

namespace fluir {
  class InspectWriter : public CodeWriter, private IndentFormatter<> {
   public:
    void writeHeader(const code::Header&, std::ostream&) override;
    void writeConstants(const std::vector<code::Value>&, std::ostream&) override;
    void writeChunk(const code::Chunk&, std::ostream&) override;

   private:
    void writeConstant(const code::Value&, std::ostream&);
    void writeCode(const code::Bytes&, std::ostream&);

    void emitInstruction(std::ostream& os, uint8_t instruction);
    void emitInstructionWithArg(std::ostream& os, uint8_t instruction, uint8_t arg);
    void emitLongArg(std::ostream& os, uint8_t arg0, uint8_t arg1, uint8_t arg2, uint8_t arg3);
  };
}  // namespace fluir

#endif
