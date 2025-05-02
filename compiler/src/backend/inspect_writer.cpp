#include "compiler/backend/inspect_writer.hpp"

#include <string>

#include "fmt/format.h"

namespace fluir {
  namespace {
#define STRINGIFY(i) #i
#define FLUIR_INSTRUCTION_TO_STR(inst) STRINGIFY(I##inst),
    std::string instructionNames[] = {
        FLUIR_CODE_INSTRUCTIONS(FLUIR_INSTRUCTION_TO_STR)};
#undef FLUIR_INSTRUCTION_TO_STR
#undef STRINGIFY
  }  // namespace

  void InspectWriter::writeHeader(const code::Header& header,
                                  std::ostream& os) {
    os << fmt::format("I{:0>2X}{:0>2X}{:0>2X}{:0>16X}\n", header.major, header.minor, header.patch,
                      header.entryOffset);
  }
  void InspectWriter::writeChunk(const code::Chunk& chunk,
                                 std::ostream& os) {
    os << fmt::format("CHUNK {}\n", chunk.name);
    indent();
    os << fmt::format("{}CONSTANTS x{:X}\n",
                      indentation(),
                      chunk.constants.size());
    // TODO: Write constants
    os << fmt::format("{}CODE x{:X}\n",
                      indentation(),
                      chunk.code.size());
    indent();
    for (const auto& byte : chunk.code) {
      os << fmt::format("{}{}\n", indentation(), instructionNames[byte]);
    }
    dedent();
    dedent();
  }

  std::string_view InspectWriter::indentation() {
    return std::string_view{indent_.c_str(), level_};
  }

  void InspectWriter::indent() {
    level_ += 2;
    if (indent_.size() < level_) {
      indent_ = std::string(level_, ' ');
    }
  }

  void InspectWriter::dedent() {
    level_ -= 2;
  }
}  // namespace fluir
