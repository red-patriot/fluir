#ifndef FLUIR_COMPILER_BACKEND_INSPECT_WRITER_HPP
#define FLUIR_COMPILER_BACKEND_INSPECT_WRITER_HPP

#include <fmt/format.h>

#include "compiler/backend/code_writer.hpp"
#include "compiler/utility/indent_formatter.hpp"

namespace fluir {
  class InspectWriter : public CodeWriter, private IndentFormatter<> {
   private:
    void writeHeader(const code::Header&, std::ostream&) override;
    void writeChunk(const code::Chunk&, std::ostream&) override;

    void writeConstants(const std::vector<code::Value>&, std::ostream&);
    void writeConstant(const code::Value&, std::ostream&);
    void writeCode(const code::Bytes&, std::ostream&);
  };
}  // namespace fluir

#endif
