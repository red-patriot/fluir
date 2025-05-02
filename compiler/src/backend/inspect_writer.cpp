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
    indent();
    for (const auto& constant : chunk.constants) {
      writeConstant(constant, os);
    }
    dedent();

    os << fmt::format("{}CODE x{:X}\n",
                      indentation(),
                      chunk.code.size());

    writeCode(chunk.code, os);

    dedent();
  }

  void InspectWriter::writeConstant(const code::Value& constant, std::ostream& os) {
    os << fmt::format("{}VFP {:.12f}\n", indentation(), constant);
  }

  void InspectWriter::writeCode(const code::Bytes& bytes, std::ostream& os) {
    indent();
    for (auto i = bytes.begin(); i != bytes.end(); ++i) {
      if (*i == code::PUSH_FP) {
        os << fmt::format("{}{} x{:X}\n", indentation(),
                          instructionNames[*i],
                          *(i + 1));
        ++i;
      } else {
        os << fmt::format("{}{}\n", indentation(),
                          instructionNames[*i]);
      }
    }
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
