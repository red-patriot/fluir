#ifndef FLUIR_COMPILER_BACKEND_INSPECT_WRITER_HPP
#define FLUIR_COMPILER_BACKEND_INSPECT_WRITER_HPP

#include "compiler/backend/code_writer.hpp"

namespace fluir {
  class InspectWriter : public CodeWriter {
   private:
    size_t level_{0};
    std::string indent_ = std::string(8, ' ');

    void writeHeader(const code::Header&, std::ostream&) override;
    void writeChunk(const code::Chunk&, std::ostream&) override;

    std::string_view indentation();
    void indent();
    void dedent();
  };
}  // namespace fluir

#endif
