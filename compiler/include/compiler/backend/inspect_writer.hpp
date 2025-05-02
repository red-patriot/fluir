#ifndef FLUIR_COMPILER_BACKEND_INSPECT_WRITER_HPP
#define FLUIR_COMPILER_BACKEND_INSPECT_WRITER_HPP

#include "compiler/backend/code_writer.hpp"
#include "fmt/format.h"

namespace fluir {
  class InspectWriter : public CodeWriter {
   private:
    size_t level_{0};
    std::string indent_ = std::string(8, ' ');

    void writeHeader(const code::Header&, std::ostream&) override;
    void writeChunk(const code::Chunk&, std::ostream&) override;

    void writeConstants(const std::vector<code::Value>&, std::ostream&);
    void writeConstant(const code::Value&, std::ostream&);
    void writeCode(const code::Bytes&, std::ostream&);

    std::string_view indentation();
    void indent();
    void dedent();

    template <typename... Args>
    std::string formatIndented(fmt::format_string<Args...> format, Args&&...);
  };
}  // namespace fluir

#endif
