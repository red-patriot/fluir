module;

#include "bytecode/byte_code.hpp"
#include "compiler/utility/indent_formatter.hpp"

export module fluir.backend.inspect_writer;
export import fluir.backend.code_writer;

namespace fluir {
  export class InspectWriter : public CodeWriter, private IndentFormatter<> {
   private:
    void writeHeader(const code::Header&, std::ostream&) override;
    void writeChunk(const code::Chunk&, std::ostream&) override;

    void writeConstants(const std::vector<code::Value>&, std::ostream&);
    void writeConstant(const code::Value&, std::ostream&);
    void writeCode(const code::Bytes&, std::ostream&);
  };
}  // namespace fluir
